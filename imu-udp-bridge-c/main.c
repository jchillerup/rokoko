#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <lo/lo.h>
#include "settings.h"

const unsigned char GET_READING_BYTE = 'g';
const unsigned char GET_IDENTIFIER_BYTE = 'i';

typedef struct sensor_args {
  char * devnode;
  int tty_fd;
  lo_address* recipient;
  char * ident;
} sensor_args;

typedef struct sensor_reading {
  char * ident;
  char * reading;
} sensor_reading;


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
  read(args->tty_fd, args->ident, 16);
  printf("Sensor ident: %s\n", args->ident);
}

void get_reading(sensor_args * args, sensor_reading* reading) {
  int read_out = 0;
  // Prepare a datastructure for receiving payloads.
  float * payload = malloc(4 * sizeof(float));

  // Write a 'g' to the Arduino
  write(args->tty_fd, &GET_READING_BYTE, 1);

  // Put the output from the Arduino in `payload'. `read' will block.
  read_out = read(args->tty_fd, payload, 16);

  if (read_out == 16) {
    memcpy(reading, payload, 4* sizeof(float));
  }
  free(payload);
}

int main(int argc,char** argv)
{
  // We allocate as many threads as we have arguments for the program, one for each device
  // to handle.
  sensor_args arg_structs[ argc - 1 ];
  int i;
  struct termios tio;
  FILE * file;
  char c;
  char * fp_recipient = calloc ( 4*3 + 3 + 1, sizeof(char));
  char * r_ptr = fp_recipient;
  int num_sensors = argc - 1;
  sensor_reading * sensors = calloc(num_sensors, sizeof(sensor_reading));
  
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
    
    prepare_sensor(&arg_structs[i-1]);
  }

  while (1) {
    // Keep reading them
    for (i=0; i<argc-1; i++) {
      get_reading(&arg_structs[i], &(sensors[i]));
    }

    // Send sensors to the recipient
    for (i = 0; i< num_sensors; i++) {
      printf("%8x ", sensors[i].reading);
    }

    printf("\n");
  }

  
  // Clean up the file descriptors
  for (i = 0; i< argc-1; i++)  {
    close(arg_structs[i].tty_fd);
  }
  
  return EXIT_SUCCESS;
}
