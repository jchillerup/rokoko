typedef struct rokoko_payload {
  float w;
  float x;
  float y;
  float z;
  short gyroX;
  short gyroY;
  short gyroZ;
  short accelX;
  short accelY;
  short accelZ;
  short magX;
  short magY;
  short magZ;
} rokoko_payload;

void * work_sensor(void * v_args) {
#ifdef ROKOKO_EXTENDED_IMU
  const unsigned char GET_READING_BYTE = 'j';
  const int PAYLOAD_LENGTH = 4*sizeof(float) + 3*3*sizeof(short);
#else
  const unsigned char GET_READING_BYTE = 'g';
  const int PAYLOAD_LENGTH = 4*sizeof(float);
#endif
  
  sensor_args * args = (sensor_args *) v_args;
  int num_packets = 0;
  int fail_packets = 0;

  int read_out = 0;

#ifdef ROKOKO_EXTENDED_IMU
  char * slug = "/sensor_accel";
#else
  char * slug = "/sensor";
#endif
  
  char * osc_address = malloc(strlen(osc_suit_id) + strlen(slug) + 1);
  // Construct the relevant osc_address
  strcpy(osc_address, osc_suit_id);
  strcpy(osc_address + strlen(osc_suit_id), slug);
  
  // Prepare a datastructure for receiving payloads.
  rokoko_payload payload;

  // wait for the other sensors to get identified before starting reading
  sleep(args->sleep_before_read);
  
  // Loop if DEBUG is false OR if it's true and num_packets < 100.
  while ( (DEBUG ^ 1) || (DEBUG && num_packets < 1000) ) {
    // Flush any data that might be in the serial buffer already
    tcflush(args->tty_fd, TCIOFLUSH);
      
    // Write a 'g' or 'j' to the Arduino
    write(args->tty_fd, &GET_READING_BYTE, 1);

#ifdef SLEEP
    // Wait for a little bit
    nanosleep(&delay_before_read, NULL);
#endif

    // Put the output from the Arduino in `payload'. `read' will block.
    read_out = read(args->tty_fd, &payload, PAYLOAD_LENGTH);
    if (read_out != PAYLOAD_LENGTH) {
      printf("Only got %d bytes\n", read_out);
      fail_packets++;
    }
      
    num_packets++;

    if (read_out < PAYLOAD_LENGTH) {
      fail_packets++;
      continue;
    }

    // Clean the data
    if (
        payload.w < -1.0 || payload.w > 1.0 ||
        payload.x < -1.0 || payload.x > 1.0 ||
        payload.y < -1.0 || payload.y > 1.0 ||
        payload.z < -1.0 || payload.z > 1.0
        ) {
      fail_packets++;
      continue;
    }
    if (DEBUG)
      printf("%.2f, %.2f, %.2f, %.2f\n", payload.x, payload.y, payload.z, payload.w);
      
    // Put the payload into an OSC message.
#ifdef ROKOKO_EXTENDED_IMU
    lo_send(*(args->recipient), osc_address, "sffffiiiiiiiii", args->sensor_ident,
            payload.x,
            payload.y,
            payload.z,
            payload.w,
            payload.gyroX,
            payload.gyroY,
            payload.gyroZ,
            payload.accelX,
            payload.accelY,
            payload.accelZ,
            payload.magX,
            payload.magY,
            payload.magZ);
#else
    lo_send(*(args->recipient), osc_address, "sffff", args->sensor_ident, payload.x, payload.y, payload.z, payload.w);
#endif
  }

  printf("num_packets: %d, fail_packets: %d\n", num_packets, fail_packets);
  
  pthread_exit(NULL);
}
