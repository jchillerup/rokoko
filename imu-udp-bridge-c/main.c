#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
 
int main(int argc,char** argv)
{
  struct termios tio;
  struct termios stdio;
  struct termios old_stdio;
  int tty_fd;
 
  unsigned char c='D';
  unsigned char GET_READING_BYTE = 'g';
  int num_packets = 0;
  
  tcgetattr(STDOUT_FILENO,&old_stdio);
 
  printf("Please start with %s /dev/ttyS1 (for example)\n",argv[0]);
  memset(&stdio,0,sizeof(stdio));
  stdio.c_iflag=0;
  stdio.c_oflag=0;
  stdio.c_cflag=0;
  stdio.c_lflag=0;
  stdio.c_cc[VMIN]=1;
  stdio.c_cc[VTIME]=0;
  tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
  tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking
 
  memset(&tio,0,sizeof(tio));
  tio.c_iflag=0;
  tio.c_oflag=0;
  tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
  tio.c_lflag=0;
  tio.c_cc[VMIN]=1;
  tio.c_cc[VTIME]=5;
 
  tty_fd=open(argv[1], O_RDWR | O_NONBLOCK);      
  cfsetospeed(&tio,B57600);            // 57600 baud
  cfsetispeed(&tio,B57600);            // 57600 baud
 
  tcsetattr(tty_fd,TCSANOW,&tio);

  char * string = malloc(100);
  int packet_done = 0;
  while (c!='q' && num_packets < 300)
    {
      int bytes = 0;      
      write(tty_fd, &GET_READING_BYTE, 1);
      packet_done = 0;
      
      // If new data is available, loop through every byte and make a UDP buffer
      while (!packet_done) {
        if (read(tty_fd, &c, 1) > 0) {
          string[bytes] = c;
          bytes++;

          if (c == '&') { // TODO: check for the \r\n also or remove it from the arduino
            num_packets++;
            packet_done = 1;
          }
        }
      }

      if (num_packets % 20 == 0) {
        printf("%d packets: %d bytes\r\n", num_packets, bytes);
        // printf("%s", string);
      }
      
      // Listen for stuff happening on stdin to quit on 'q'
      read(STDIN_FILENO, &c, 1);
      
    }

  free(string);
  close(tty_fd);
  tcsetattr(STDOUT_FILENO,TCSANOW,&old_stdio);
 
  return EXIT_SUCCESS;
}
