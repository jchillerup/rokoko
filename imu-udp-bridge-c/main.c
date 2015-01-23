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

#include <nanomsg/nn.h>
#include <nanomsg/tcp.h>
#include <nanomsg/pipeline.h>

#define ROKOKO_IMU 1
#define ROKOKO_COLOR 2

char * osc_suit_id;
char * osc_ip;
const char* broker_url = "ipc:///tmp/rokoko-socket.ipc";
int broker_socket;
struct timespec delay_before_read;

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
  char sensor_ident[16];
  int sensor_type;
} sensor_args;

// TODO: make these includes a bit more proper
#include "prepare_sensor.c"
#include "work_sensor.c"
#include "work_glove.c"

int main(int argc,char** argv)
{
  // We allocate as many threads as we have arguments for the program, one for each device
  // to handle.
  pthread_t threads[ argc - 1 ];
  sensor_args arg_structs[ argc - 1 ];
  int i;
  struct termios tio;

  if (argc <= 3) {
    printf("No device node(s) given\n");
    return(255);
  }

  osc_ip = argv[1];
  osc_suit_id = argv[2];

  // Set up a socket for the broker
  broker_socket = nn_socket(AF_SP, NN_PUSH);
  nn_connect(broker_socket, broker_url);
  
  printf("ROKOKO positure and color streamer starting, sending to osc://%s%s\n", osc_ip, osc_suit_id);  
  prepare_terminal(&tio);

  // Set up a delay time between reads
  delay_before_read.tv_sec = 0;
  delay_before_read.tv_nsec = 8 * 1e6; // 1e6 nanoseconds = 1 millisecond
  
  // Loop through all commandline args and start a thread for each sensor we
  // want to look at.
  for (i = 3; i < argc; i++) {
    int fd = open_device(argv[i], &tio);
    lo_address recipient = lo_address_new(osc_ip, "14040");
    int sensor_type;
    
    // Construct the arguments struct and spawn a thread
    //sensor_args * args = malloc(sizeof(sensor_args));
    arg_structs[i-3].devnode = argv[i];
    arg_structs[i-3].tty_fd = fd;
    arg_structs[i-3].recipient = &recipient;
    arg_structs[i-3].sleep_before_read = argc - (i - 3) + 1;

    sensor_type = prepare_sensor(&arg_structs[i-3]);
    
    switch (sensor_type ) {
    case ROKOKO_IMU:
      pthread_create(&threads[i-3], NULL, work_sensor, &arg_structs[i-3]);
      break;
    case ROKOKO_COLOR:
      pthread_create(&threads[i-3], NULL, work_glove, &arg_structs[i-3]);
      break;
    default:
      printf("Could not determine sensor type for: TODO (%d)\n", sensor_type);
      break;
    }

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
