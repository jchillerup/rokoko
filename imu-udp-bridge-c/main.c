#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <lo/lo.h>
#include "settings.h"

int main(int argc,char** argv)
{
  struct termios tio;
  int tty_fd;
  
  const unsigned char GET_READING_BYTE = 'g';
  const unsigned char GET_IDENTIFIER_BYTE = 'i';
  
  int num_packets = 0;
  int fail_packets = 0;
  
  if (argc == 1) {
    printf("No device node given\n");
    return(255);
  }
  
  char* sensor_devnode = argv[1];
  char sensor_ident[16];
  
  memset(&tio,0,sizeof(tio));
  tio.c_iflag     = 0;
  tio.c_oflag     = 0;
  tio.c_cflag     = CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
  tio.c_lflag     = 0;
  tio.c_cc[VMIN]  = 16;
  tio.c_cc[VTIME] = 5;

  tty_fd = open(sensor_devnode, O_RDWR | O_NOCTTY);
  cfsetospeed(&tio, B115200);            // 115200 baud
  cfsetispeed(&tio, B115200);            // 115200 baud
  tcsetattr(tty_fd, TCSANOW, &tio);
  
  float * payload = malloc(4 * sizeof(float));
  lo_address recipient = lo_address_new(RECIPIENT, "14040");

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
      
      // BUG: Sometimes the readings come out wrong. We filter these out (but really, the bug should be fixed)
      /* if ( */
      /*     payload[0] > 1.0 || payload[0] < -1.0 || */
      /*     payload[1] > 1.0 || payload[1] < -1.0 || */
      /*     payload[2] > 1.0 || payload[2] < -1.0 || */
      /*     payload[3] > 1.0 || payload[3] < -1.0 */
      /*     ) { */
      /*   fail_packets++; */
      /*   continue; */
      /* } */

      if (DEBUG)
        printf("%.2f, %.2f, %.2f, %.2f\n", payload[1], payload[2], payload[3], payload[0]);

      // Put the payload into an OSC message...
      lo_send(recipient, "/sensor", "sffff", sensor_ident, payload[1], payload[2], payload[3], payload[0]);
    }

  printf("num_packets: %d, fail_packets: %d\n", num_packets, fail_packets);

  free(payload);
  close(tty_fd);

  return EXIT_SUCCESS;
}
