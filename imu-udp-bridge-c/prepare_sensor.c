int prepare_sensor (void * v_args) {
  int i;
  sensor_args * args = (sensor_args *) v_args;
  
  const unsigned char GET_IDENTIFIER_BYTE = 'i';
  const unsigned char GET_TYPE_BYTE = 't';
    
  char * sensor_ident = args->sensor_ident;
  char sensor_type_string[16];
  
  // Wait a second after opening the device for Arduino Unos to have
  // their initialization done.
  sleep(2);

  // Flush any data that might be in the serial buffer already
  tcflush(args->tty_fd, TCIOFLUSH);

  sleep(1);

  //while( ~((sensor_ident[0] >= '0' && sensor_ident[0] <= '9') || (sensor_ident[0] >= 'A' && sensor_ident[0] <= 'Z')) )  {
  while(sensor_ident[0] < 0x30 || sensor_ident[0] > 0x5a) {
    // Get the ident of the sensor
    write(args->tty_fd, &GET_IDENTIFIER_BYTE, 1);
#ifdef SLEEP
    nanosleep(&delay_before_read, NULL);
#endif
    read(args->tty_fd, args->sensor_ident, 16);
  }
  
#ifdef SLEEP
  nanosleep(&delay_before_read, NULL);
#endif
  
  while(sensor_type_string[0] < 0x30 || sensor_type_string[0] > 0x5a) {
    // Get the sensor type
    write(args->tty_fd, &GET_TYPE_BYTE, 1);
#ifdef SLEEP
    nanosleep(&delay_before_read, NULL);
#endif
    read(args->tty_fd, sensor_type_string, 16);
  }
  
  // clear out some of the bytes of the name
  for (i=2; i<16; i++) {
    sensor_ident[i] = 0;
  }
 
  printf("%s: %s\n", args->devnode, sensor_ident);

  if (sensor_type_string[0] == 'I') {
    args->sensor_type = ROKOKO_IMU;
    return ROKOKO_IMU;
  } else if(sensor_type_string[0] == 'C') {
    args->sensor_type = ROKOKO_COLOR;
    return ROKOKO_COLOR;
  }

  return -1;
}
