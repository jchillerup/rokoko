////////////////////////////////////////////////////////////////////////////
//
//  This file is part of MPU9150Lib
//
//  Copyright (c) 2013 Pansenti, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//  This example sketch shows how to calculate residual accelerations in the body frame,
//  compensated for gravity.

#include <Wire.h>
#include "I2Cdev.h"
#include "MPU9150Lib.h"
#include "CalLib.h"
#include <dmpKey.h>
#include <dmpmap.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>
#include <EEPROM.h>

MPU9150Lib MPU;                                              // the MPU object

//  MPU_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the sensor data and DMP output

#define MPU_UPDATE_RATE  (20)

//  MAG_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the magnetometer data
//  MAG_UPDATE_RATE should be less than or equal to the MPU_UPDATE_RATE

#define MAG_UPDATE_RATE  (10)

//  MPU_MAG_MIX defines the influence that the magnetometer has on the yaw output.
//  The magnetometer itself is quite noisy so some mixing with the gyro yaw can help
//  significantly. Some example values are defined below:

#define  MPU_MAG_MIX_GYRO_ONLY          0                  // just use gyro yaw
#define  MPU_MAG_MIX_MAG_ONLY           1                  // just use magnetometer and no gyro yaw
#define  MPU_MAG_MIX_GYRO_AND_MAG       10                 // a good mix value 
#define  MPU_MAG_MIX_GYRO_AND_SOME_MAG  50                 // mainly gyros with a bit of mag correction 

//  MPU_LPF_RATE is the low pas filter rate and can be between 5 and 188Hz
#define MPU_LPF_RATE   5

//  SERIAL_PORT_SPEED defines the speed to use for the debug serial port
#define  SERIAL_PORT_SPEED  57600

MPUQuaternion gravity;                                     // this is our earth frame gravity vector

double w = 0.3, x = 0, y = 0, z = -0.95;
double xx = 0, yy = 0, zz = 0;

void setup()
{
  Serial.begin(SERIAL_PORT_SPEED);
  Serial.println("Accel9150 starting");

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  delay(5);
}

void loop()
{
  Serial.write("DS");
  Serial.write("\n");

  // the fused quaternion
  Serial.print(w += 0.04); Serial.write("!");
  Serial.print(x += 0.01); Serial.write("!");
  Serial.print(y += 0.02); Serial.write("!");
  Serial.print(z += 0.00);

  Serial.print("\n");

  Serial.print(xx); Serial.write("!");
  Serial.print(yy); Serial.write("!");
  Serial.print(zz);

  if ( w > 1.0 ) w -= 2.0;
  if ( x > 1.0 ) x -= 2.0;
  if ( y > 1.0 ) y -= 2.0;
  if ( z > 1.0 ) z -= 2.0;
  

  Serial.write("&");
  
  delay(30);
}

