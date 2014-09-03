#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <lo/lo.h>
#include <pthread.h>
#include <time.h>
#include "settings.h"

#define DEBUG 1
char * osc_address;
char * osc_ip;

void prepare_terminal(struct termios * tio) {
  memset(tio, 0, sizeof(*tio));
  tio->c_iflag     = 0;
  tio->c_oflag     = 0;
  tio->c_cflag     = CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
  tio->c_lflag     &= ~ICANON;
  tio->c_cc[VMIN]  = 16;
  tio->c_cc[VTIME] = 5;
}

// Sets up a device for dumping data. Returns a file descriptor
int open_device(char* address, struct termios * tio) {
  int tty_fd;

  tty_fd = open(address, O_RDWR | O_NOCTTY | O_NONBLOCK);
  cfsetospeed(tio, B9600);            // 115200 baud
  cfsetispeed(tio, B9600);            // 115200 baud
  tcsetattr(tty_fd, TCSANOW, tio);

  return tty_fd;
}

typedef struct sensor_args {
  char * devnode;
  int tty_fd;
  lo_address* recipient;
  int sleep_before_read;
} sensor_args;


void * work_sensor(void * v_args) {
  int i;
  sensor_args * args = (sensor_args *) v_args;
  
  const unsigned char GET_READING_BYTE = 'g';
  const unsigned char GET_IDENTIFIER_BYTE = 'i';
  
  int num_packets = 0;
  int fail_packets = 0;
  char sensor_ident[16];

  // Set up a delay time between reads
  struct timespec delay_before_read;
  int read_out = 0;
  delay_before_read.tv_sec = 0;
  delay_before_read.tv_nsec = 8 * 1e6; // 1e6 nanoseconds = 1 millisecond

  // Prepare a datastructure for receiving payloads.
  float * payload = malloc(4 * sizeof(float));

  // Wait a second after opening the device for Arduino Unos to have
  // their initialization done.
  sleep(3);

  // Flush any data that might be in the serial buffer already
  tcflush(args->tty_fd, TCIOFLUSH);

  sleep(3);
  
  // Get the ident of the sensor
  write(args->tty_fd, &GET_IDENTIFIER_BYTE, 1);
#ifdef SLEEP
  nanosleep(&delay_before_read, NULL);
#endif
  read(args->tty_fd, sensor_ident, 16);

  // clear out some of the bytes of the name
  for (i=2; i<16; i++) {
    sensor_ident[i] = 0;
  }
  
  printf("%s: %s\n", args->devnode, sensor_ident);

  // wait for the other sensors to get identified before starting reading
  sleep(args->sleep_before_read);
  
  // Loop if DEBUG is false OR if it's true and num_packets < 100.
  while ( (DEBUG ^ 1) || (DEBUG && num_packets < 1000) )
    {
      // Flush any data that might be in the serial buffer already
      tcflush(args->tty_fd, TCIOFLUSH);
      
      // Write a 'g' to the Arduino
      write(args->tty_fd, &GET_READING_BYTE, 1);

#ifdef SLEEP
      // Wait for a little bit
      nanosleep(&delay_before_read, NULL);
#endif

      // Put the output from the Arduino in `payload'. `read' will block.
      read_out = read(args->tty_fd, payload, 16);
      if (read_out != 16) continue;
      
      num_packets++;

      if (read_out < 16) {
        fail_packets++;
        continue;
      }

      // Clean the data
      if (
          payload[0] < -1.0 || payload[0] > 1.0 ||
          payload[1] < -1.0 || payload[1] > 1.0 ||
          payload[2] < -1.0 || payload[2] > 1.0 ||
          payload[3] < -1.0 || payload[3] > 1.0
          ) {
        fail_packets++;
        continue;
      }
      if (DEBUG)
        printf("%.2f, %.2f, %.2f, %.2f\n", payload[1], payload[2], payload[3], payload[0]);
      
      // Put the payload into an OSC message.
      lo_send(*(args->recipient), osc_address, "sffff", sensor_ident, payload[1], payload[2], payload[3], payload[0]);
    }

  printf("num_packets: %d, fail_packets: %d\n", num_packets, fail_packets);
  
  free(payload);

  pthread_exit(NULL);
}


int main(int argc,char** argv)
{
  // We allocate as many threads as we have arguments for the program, one for each device
  // to handle.
  pthread_t threads[ argc - 1 ];
  sensor_args arg_structs[ argc - 1 ];
  int i;
  struct termios tio;
  FILE * file;
  char c;
  char * fp_recipient = calloc ( 4*3 + 3 + 1, sizeof(char));
  char * r_ptr = fp_recipient;

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

  if (argc <= 3) {
    printf("No device node(s) given\n");
    return(255);
  }

  osc_ip = argv[1];
  osc_address = argv[2];
  
  printf("ROKOKO positure streamer starting, sending to osc://%s%s\n", osc_ip, osc_address);  

  prepare_terminal(&tio);

  // Loop through all commandline args and start a thread for each sensor we
  // want to look at.
  for (i = 3; i < argc; i++) {
    int fd = open_device(argv[i], &tio);
    lo_address recipient = lo_address_new(osc_ip, "14040");
    
    // Construct the arguments struct and spawn a thread
    //sensor_args * args = malloc(sizeof(sensor_args));
    arg_structs[i-3].devnode = argv[i];
    arg_structs[i-3].tty_fd = fd;
    arg_structs[i-3].recipient = &recipient;
    arg_structs[i-3].sleep_before_read = argc-(i-3)+1;

    pthread_create(&threads[i-3], NULL, work_sensor, &arg_structs[i-3]);

    // Block for a second so the sensors take turns in starting
    sleep(1);
  }

  for (i = 0; i < argc-3; i++) {
    pthread_join(threads[i], NULL);
  }
  
  // Clean up the file descriptors
  for (i = 0; i< argc-1; i++)  {
    close(arg_structs[i].tty_fd);
  }
  
  return EXIT_SUCCESS;
}
