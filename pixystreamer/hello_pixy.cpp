//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "pixy.h"
#include <lo/lo.h>

#define BLOCK_BUFFER_SIZE    25

// Pixy Block buffer // 
struct Block blocks[BLOCK_BUFFER_SIZE];

void handle_SIGINT(int unused)
{
  // On CTRL+C - abort! //

  printf("\nBye!\n");
  exit(0);
}

int main(int argc, char * argv[])
{
  int      index;
  int      blocks_copied;
  int      pixy_init_status;

  // Catch CTRL+C (SIGINT) signals //
  signal(SIGINT, handle_SIGINT);

  printf("Rokoko streamer:\n libpixyusb Version: %s\n", __LIBPIXY_VERSION__);

  // Connect to Pixy //
  pixy_init_status = pixy_init();

  // Was there an error initializing pixy? //
  if(!pixy_init_status == 0)
  {
    // Error initializing Pixy //
    printf("pixy_init(): ");
    pixy_error(pixy_init_status);

    return pixy_init_status;
  }

  // Request Pixy firmware version //
  {
    uint16_t major;
    uint16_t minor;
    uint16_t build;
    int      return_value;

    return_value = pixy_get_firmware_version(&major, &minor, &build);

    if (return_value) {
      // Error //
      printf("Failed to retrieve Pixy firmware version. ");
      pixy_error(return_value);

      return return_value;
    } else {
      // Success //
      printf(" Pixy Firmware Version: %d.%d.%d\n", major, minor, build);
    }
  }

  // Pixy Command Examples //
  {
    int32_t response;
    int     return_value;

    // Execute remote procedure call "cam_setAWB" with one output (host->pixy) parameter (Value = 1)
    //
    //   Parameters:                 Notes:
    //
    //   pixy_command("cam_setAWB",  String identifier for remote procedure
    //                        0x01,  Length (in bytes) of first output parameter
    //                           1,  Value of first output parameter
    //                           0,  Parameter list seperator token (See value of: END_OUT_ARGS)
    //                   &response,  Pointer to memory address for return value from remote procedure call
    //                           0); Parameter list seperator token (See value of: END_IN_ARGS)
    //

    // Enable auto white balance //
    return_value = pixy_command("cam_setAWB", 0x01, 1, 0, &response, 0);

    // Execute remote procedure call "cam_getAWB" with no output (host->pixy) parameters
    //
    //   Parameters:                 Notes:
    //
    //   pixy_command("cam_setAWB",  String identifier for remote procedure
    //                           0,  Parameter list seperator token (See value of: END_OUT_ARGS)
    //                   &response,  Pointer to memory address for return value from remote procedure call
    //                           0); Parameter list seperator token (See value of: END_IN_ARGS)
    //

    // Get auto white balance //
    return_value = pixy_command("cam_getAWB", 0, &response, 0);

    // Set auto white balance back to disabled //
    pixy_command("cam_setAWB", 0x01, 0, 0, &response, 0);
  }

  printf("Detecting blocks...\n");

  for(;;)
  {
    // Get blocks from Pixy //
    blocks_copied = pixy_get_blocks(BLOCK_BUFFER_SIZE, &blocks[0]);

    if(blocks_copied < 0) {
      // Error: pixy_get_blocks //
      printf("pixy_get_blocks(): ");
      pixy_error(blocks_copied);
      usleep(250000);
    }

    // Display received blocks //
    for(index = 0; index != blocks_copied; ++index) {

      switch (blocks[index].type) {
        case TYPE_NORMAL:
          printf("[sig:%2u w:%3u h:%3u x:%3u y:%3u]\n",
                 blocks[index].signature,
                 blocks[index].width,
                 blocks[index].height,
                 blocks[index].x,
                 blocks[index].y);
        break;

        case TYPE_COLOR_CODE:
          printf("[sig:%2u w:%3u h:%3u x:%3u y:%3u ang:%3i]\n",
                 blocks[index].signature,
                 blocks[index].width,
                 blocks[index].height,
                 blocks[index].x,
                 blocks[index].y,
                 blocks[index].angle);
        break;
      }
    }
  }
}
