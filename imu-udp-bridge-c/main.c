#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <lo/lo.h>
#include <pthread.h>
#include "settings.h"

// Sets up a device for dumping data. Returns a file descriptor
int open_device(char* address, struct termios * tio) {
  int tty_fd;

  tty_fd = open(address, O_RDWR | O_NOCTTY);
  cfsetospeed(tio, B115200);            // 115200 baud
  cfsetispeed(tio, B115200);            // 115200 baud
  tcsetattr(tty_fd, TCSANOW, tio);

  return tty_fd;
}

void work_sensor(int tty_fd, lo_address* recipient) {
  const unsigned char GET_READING_BYTE = 'g';
  const unsigned char GET_IDENTIFIER_BYTE = 'i';
  
  int num_packets = 0;
  int fail_packets = 0;
  char sensor_ident[16];

  // Prepare a datastructure for receiving payloads
  float * payload = malloc(4 * sizeof(float));

  // Get the ident of the sensor
  write(tty_fd, &GET_IDENTIFIER_BYTE, 1);
  assert(read(tty_fd, sensor_ident, 16) == 16);
  printf("Sensor ident: %s\n", sensor_ident);

  // Loop if DEBUG is false OR if it's true and num_packets < 100
  while ( (DEBUG ^ 1) || (DEBUG && num_packets < 100) )
    {
      int read_out = 0;
      // Write a 'g' to the Arduino
      write(tty_fd, &GET_READING_BYTE, 1);

      // Put the output from the Arduino in `payload'. `read' will block.
      read_out = read(tty_fd, payload, 16);
      num_packets++;

      if (read_out < 16) {
        printf("short read\n");
        fail_packets++;
        continue;
      }
      
      if (DEBUG)
        printf("%.2f, %.2f, %.2f, %.2f\n", payload[1], payload[2], payload[3], payload[0]);

      // Put the payload into an OSC message...
      lo_send(recipient, "/sensor", "sffff", sensor_ident, payload[1], payload[2], payload[3], payload[0]);
    }

  printf("num_packets: %d, fail_packets: %d\n", num_packets, fail_packets);
  
  free(payload);
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

int main(int argc,char** argv)
{
  // We allocate as many threads as we have arguments for the program, one for each device
  // to handle.
  pthread_t threads[ argc - 1 ];
  int i;
  struct termios tio;
  
  if (argc == 1) {
    printf("No device node(s) given\n");
    return(255);
  }
  
  prepare_terminal(&tio);
  
  for (i = 1; i < argc; i++) {
    printf("Opening %s\n", argv[i]);
    
    int fd               = open_device(argv[i], &tio);
    lo_address recipient = lo_address_new(RECIPIENT, "14040");
    work_sensor(fd, &recipient);
    close(fd);
  }
  
  return EXIT_SUCCESS;
}
