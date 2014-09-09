void * work_glove(void * v_args) {
  char sensor_ident[16];
  sensor_args * args = (sensor_args *) v_args;
  struct timespec delay_before_read;
  int read_out = 0;
  
  const unsigned char GET_READING_BYTE = 'g';
  
  // Prepare a datastructure for receiving payloads.
  uint16_t * payload = malloc(4 * sizeof(uint16_t) * 2);
  
  // Establish the proper OSC address to send to
  char * osc_address = malloc(strlen(osc_suit_id) + strlen("/glove") + 1);
  strcpy(osc_address, osc_suit_id);
  strcpy(osc_address + strlen(osc_suit_id), "/glove");
  
  // wait for the other sensors to get identified before starting reading
  sleep(args->sleep_before_read);
  
  while (1) {
    read_out = 0;
    
    // Flush any data that might be in the serial buffer already
    tcflush(args->tty_fd, TCIOFLUSH);
    
    // Write a 'g' to the Arduino
    write(args->tty_fd, &GET_READING_BYTE, 1);

    #ifdef SLEEP
    // Wait for a little bit
    //nanosleep(&delay_before_read, NULL);
    usleep(100*1000);
    #endif
    
    // Put the output from the Arduino in `payload'. `read' will block.
    while(read_out != 16) {
      read_out = read(args->tty_fd, payload, 16);
      nanosleep(&delay_before_read, NULL);
    }

    payload[0] ^= 0b10101010;
    payload[1] ^= 0b10101010;
    payload[2] ^= 0b10101010;
    payload[3] ^= 0b10101010;
    
    
    //printf("R: %d G: %d B: %d C: %d\n", payload[0] ^ 0b10101010, payload[1] ^ 0b10101010, payload[2] ^ 0b10101010, payload[3] ^ 0b10101010);

    // Put the payload into an OSC message.
    lo_send(*(args->recipient), osc_address, "siiii", args->sensor_ident, payload[0], payload[1], payload[2], payload[3]);
  }

  free(payload);
  
  pthread_exit(NULL);
}
