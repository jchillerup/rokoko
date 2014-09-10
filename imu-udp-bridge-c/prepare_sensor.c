int prepare_sensor (void * v_args) {
  sensor_args * args = (sensor_args *) v_args;
  
  const unsigned char GET_IDENTIFIER_BYTE = 'i';
  const unsigned char GET_TYPE_BYTE = 't';
  int i = 0;

  char * sensor_ident = args->sensor_ident;
  char sensor_type_string[16];
  
  // Wait a second after opening the device for Arduino Unos to have
  // their initialization done.
  sleep(2);

  // Flush any data that might be in the serial buffer already
  tcflush(args->tty_fd, TCIOFLUSH);

  sleep(2);

  //while( ~((sensor_ident[0] >= '0' && sensor_ident[0] <= '9') || (sensor_ident[0] >= 'A' && sensor_ident[0] <= 'Z')) )  {
  while(i == 0 || (sensor_ident[0] != 'I' && sensor_ident[0] != 'C')) {
    // Get the ident of the sensor
    write(args->tty_fd, &GET_IDENTIFIER_BYTE, 1);
#ifdef SLEEP
    nanosleep(&delay_before_read, NULL);
#endif
    read(args->tty_fd, sensor_ident, 16);

    i++;
  }
  
#ifdef SLEEP
  nanosleep(&delay_before_read, NULL);
#endif
  
  i=0;
  while(i == 0 || sensor_type_string[0] < 0x30 || sensor_type_string[0] > 0x5a) {
    // Get the sensor type
    write(args->tty_fd, &GET_TYPE_BYTE, 1);
#ifdef SLEEP
    nanosleep(&delay_before_read, NULL);
#endif
    read(args->tty_fd, sensor_type_string, 16);

    i++;
  }
  
  // clear out some of the bytes of the name

  for (i=6; i<16; i++) {
    sensor_type_string[i] = 0;
  }

  for (i=2; i<16; i++) {
    sensor_ident[i] = 0;
  }
 
  printf("%s: %s (%s)\n", args->devnode, sensor_ident, sensor_type_string);

  if (sensor_type_string[0] == 'I') {
    args->sensor_type = ROKOKO_IMU;
    return ROKOKO_IMU;
  } else if(sensor_type_string[0] == 'C') {
    args->sensor_type = ROKOKO_COLOR;
    return ROKOKO_COLOR;
  }

  return -1;
}
