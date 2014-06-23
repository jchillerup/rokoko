#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <lo/lo.h>
#include "settings.h"
#include <time.h>
#include <pthread.h>

const unsigned char GET_READING_BYTE = 'g';
const unsigned char GET_IDENTIFIER_BYTE = 'i';


typedef struct sensor_reading {
  char * ident;
  float w;
  float x;
  float y;
  float z;
} sensor_reading;

typedef struct sensor_args {
  char * devnode;
  int tty_fd;
  lo_address* recipient;
  char * ident;
  sensor_reading reading;
} sensor_args;


struct timespec rw_delay;
struct timespec net_delay;

// Sets up a device for dumping data. Returns a file descriptor
int open_device(char* address, struct termios * tio) {
  int tty_fd;

  tty_fd = open(address, O_RDWR | O_NOCTTY );
  cfsetospeed(tio, B115200);            // 115200 baud
  cfsetispeed(tio, B115200);            // 115200 baud
  tcsetattr(tty_fd, TCSANOW, tio);

  return tty_fd;
}

void prepare_terminal(struct termios * tio) {
  memset(tio, 0, sizeof(*tio));
  tio->c_iflag     = 0;
  tio->c_oflag     = 0;
  tio->c_cflag     = CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
  tio->c_lflag     = 0;
  tio->c_cc[VMIN]  = 16;
  tio->c_cc[VTIME] = 5;
}

void * prepare_sensor(sensor_args * args) {  
  int num_packets = 0;
  int fail_packets = 0;

  // Get the ident of the sensor
  write(args->tty_fd, &GET_IDENTIFIER_BYTE, 1);

  // Sleep for a while
  nanosleep(&rw_delay, NULL);

  read(args->tty_fd, args->ident, 16);
  printf("Sensor ident: %s\n", args->ident);
}

void * get_reading(void * v_args) {
  int read_out = 0;
  sensor_args * args = (sensor_args*) v_args;
    
  // Prepare a datastructure for receiving payloads.
  float * payload = malloc(4 * sizeof(float));

  while(1) {
    // Write a 'g' to the Arduino
    write(args->tty_fd, &GET_READING_BYTE, 1);

    // Sleep for a while
    nanosleep(&rw_delay, NULL);
  
    // Put the output from the Arduino in `payload'. `read' will block.
    read_out = read(args->tty_fd, payload, 16);
    
    if (read_out == 16) {
      args->reading.w = payload[0];
      args->reading.x = payload[1];
      args->reading.y = payload[2];
      args->reading.z = payload[3];

      //reading->ident = args->ident;
    }
  }
  free(payload);
}

int main(int argc,char** argv)
{
  // We allocate as many threads as we have arguments for the program, one for each device
  // to handle.
  sensor_args arg_structs[ argc - 1 ];
  int i, num_packets;
  struct termios tio;
  FILE * file;
  char c;
  char * fp_recipient = calloc ( 4*3 + 3 + 1, sizeof(char));
  char * r_ptr = fp_recipient;
  int num_sensors = argc - 1;
  sensor_reading * sensors = calloc(num_sensors, sizeof(sensor_reading));
  pthread_t threads[argc-1];
  
  // Set the delay to wait before polling for an answer
  rw_delay.tv_sec = 0;
  rw_delay.tv_nsec = 8 * 1e6; // 1000000 ns = 1ms

  // Set the delay between network packets being sent
  net_delay.tv_sec = 0;
  net_delay.tv_nsec = 50*1e6;
  
  // Read the IP of the recipient from recipient.txt
  file = fopen( "recipient.txt" , "r");
  if (file) {
    while ((c = getc(file)) != EOF && c != '\n' && c != '\r' && !feof(file)) {
      *(r_ptr++) = c;
    }
    *(r_ptr) = 0;
  } else {
    printf("recipient.txt missing.\n");
    return(255);
  }

  if (argc == 1) {
    printf("No device node(s) given\n");
    return(255);
  }
  
  printf("ROKOKO streamer starting, sending to %s\n", fp_recipient);  
  printf("Allocating %d sensors\n", num_sensors);
  
  prepare_terminal(&tio);

  // Loop through all commandline args and prepare each sensor
  for (i = 1; i < argc; i++) {
    printf("Opening %s\n", argv[i]);

    int fd = open_device(argv[i], &tio);
    lo_address recipient = lo_address_new(fp_recipient, "14040");
    
    // Construct the arguments struct and spawn a thread
    //sensor_args * args = malloc(sizeof(sensor_args));
    arg_structs[i-1].devnode = argv[i];
    arg_structs[i-1].tty_fd = fd;
    arg_structs[i-1].recipient = &recipient;
    arg_structs[i-1].ident = calloc(16, sizeof(char));
    arg_structs[i-1].reading = sensors[i-1];
    
    prepare_sensor(&arg_structs[i-1]);
  }

  // Keep reading them
  for (i = 0; i < argc-1; i++) {
    pthread_create(&threads[i], NULL, get_reading, &arg_structs[i]);
  }

  while (1) {
    // Create a packet with all the data

    for(i=0; i< argc-1; i++) {
      lo_send(*(arg_structs[i-1].recipient), "/sensor", "sffff", arg_structs[i].ident, arg_structs[i].reading.x, arg_structs[i].reading.y, arg_structs[i].reading.z, arg_structs[i].reading.w);
      //printf("%s %.02f %.02f %.02f %.02f\n", arg_structs[i].ident, arg_structs[i].reading.w, arg_structs[i].reading.x,  arg_structs[i].reading.y, arg_structs[i].reading.z);
    }
 
    printf(".\n");   
    nanosleep(&net_delay, NULL);
  }

  for (i=0; i< argc-1; i++) {
    pthread_join(threads[i], NULL);
  }
  
  
  // Clean up the file descriptors
  for (i = 0; i< argc-1; i++)  {
    close(arg_structs[i].tty_fd);
  }
  
  return EXIT_SUCCESS;
}
