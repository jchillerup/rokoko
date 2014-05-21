#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <lo/lo.h>

#define RECIPIENT "192.168.1.50"


int main(int argc,char** argv)
{
  struct termios tio;
  int tty_fd;
  
  unsigned char GET_READING_BYTE = 'g';
  int num_packets = 0;
  int fail_packets = 0;

  if (argc == 1) {
    printf("No device node given\n");
    return(255);
  }
  
  char* sensor_devnode = argv[1];
  
  memset(&tio,0,sizeof(tio));
  tio.c_iflag     = 0;
  tio.c_oflag     = 0;
  tio.c_cflag     = CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
  tio.c_lflag     = 0;
  tio.c_cc[VMIN]  = 1;
  tio.c_cc[VTIME] = 5;

  tty_fd = open(sensor_devnode, O_RDWR);
  cfsetospeed(&tio, B9600);            // 9600 baud
  cfsetispeed(&tio, B9600);            // 9600 baud

  tcsetattr(tty_fd, TCSANOW, &tio);

  float * payload = malloc(4 * sizeof(float));
  while (1)
    {
      // Write a 'g' to the Arduino
      write(tty_fd, &GET_READING_BYTE, 1);

      // Put the output from the Arduino in `payload'. `read' will block.
      read(tty_fd, payload, 16);
      num_packets++;

      // BUG: Sometimes the readings come out wrong. We filter these out (but really, the bug should be fixed)
      if (
          payload[0] > 1.0 || payload[0] < -1.0 ||
          payload[1] > 1.0 || payload[1] < -1.0 ||
          payload[2] > 1.0 || payload[2] < -1.0 ||
          payload[3] > 1.0 || payload[3] < -1.0
          ) {
        fail_packets++;
        continue;
      }

      // Put the payload into an OSC message...
      //lo_blob payload_blob = lo_blob_new(sizeof(payload), payload);
      lo_address recipient = lo_address_new(RECIPIENT, "14040");
      lo_send(recipient, "/sensor", "sffff", sensor_devnode, payload[1], payload[2], payload[3], payload[0]);
      
    }

  printf("num_packets: %d, fail_packets: %d\n", num_packets, fail_packets);

  free(payload);
  close(tty_fd);

  return EXIT_SUCCESS;
}
