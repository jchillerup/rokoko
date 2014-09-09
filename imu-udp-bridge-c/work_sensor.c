void * work_sensor(void * v_args) {
  const unsigned char GET_READING_BYTE = 'g';
  sensor_args * args = (sensor_args *) v_args;
  int num_packets = 0;
  int fail_packets = 0;

  int read_out = 0;
 
  char * osc_address = malloc(strlen(osc_suit_id) + strlen("/sensor") + 1);
  
  // Construct the relevant osc_address
  strcpy(osc_address, osc_suit_id);
  strcpy(osc_address + strlen(osc_suit_id), "/sensor");
  
  // Prepare a datastructure for receiving payloads.
  float * payload = malloc(4 * sizeof(float));

  // wait for the other sensors to get identified before starting reading
  sleep(args->sleep_before_read);
  
  // Loop if DEBUG is false OR if it's true and num_packets < 100.
  while ( (DEBUG ^ 1) || (DEBUG && num_packets < 1000) ) {
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
    lo_send(*(args->recipient), osc_address, "sffff", args->sensor_ident, payload[1], payload[2], payload[3], payload[0]);
  }

  printf("num_packets: %d, fail_packets: %d\n", num_packets, fail_packets);
  
  free(payload);

  pthread_exit(NULL);
}
