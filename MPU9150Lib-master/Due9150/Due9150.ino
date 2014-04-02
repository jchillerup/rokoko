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

//  The Due version has built in tools for accela nd mag calibration. While in normal mode,
//  enter:
//
//  'a' - enter accel calibration mode.
//  'm' - enter mag calibration mode.
//
//  The calibration routines behave just like the individual sketches for AVR Arduinos.
//  Enter 's' to save the data, 'x' to exit. So, move the MPU-9150 around as normal for caliibration
//  and then enter 's' to save the data, followed by 'x'. If something went wrong, just enter 'x'
//  to discard the data.
//
//  *** Inportant Note ***
//
//  The calibration data is stored in flash and is overwritten every time a new sketch is uploaded.

#include <Wire.h>
#include "I2Cdev.h"
#include "MPU9150Lib.h"
#include "CalLib.h"
#include "DueFlash.h"
#include <dmpKey.h>
#include <dmpmap.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>

//  DEVICE_TO_USE selects whether the IMU at address 0x68 (default) or 0x69 is used
//    0 = use the device at 0x68
//    1 = use the device at ox69

#define  DEVICE_TO_USE    0

MPU9150Lib dueMPU;                                              // the MPU object

//  MPU_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the sensor data and DMP output

#define MPU_UPDATE_RATE  (20)

//  MAG_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the magnetometer data
//  MAG_UPDATE_RATE should be less than or equal to the MPU_UPDATE_RATE

#define MAG_UPDATE_RATE  (10)

//  MPU_MAG_MIX defines the influence that the magnetometer has on the yaw output.
//  The magnetometer itself is quite noisy so some mixing with the gyro yaw can help
//  significantly. Some example values are defined below:

#define  MPU_MAG_MIX_GYRO_ONLY          0                   // just use gyro yaw
#define  MPU_MAG_MIX_MAG_ONLY           1                   // just use magnetometer and no gyro yaw
#define  MPU_MAG_MIX_GYRO_AND_MAG       10                  // a good mix value 
#define  MPU_MAG_MIX_GYRO_AND_SOME_MAG  50                  // mainly gyros with a bit of mag correction 

//  MPU_LPF_RATE is the low pas filter rate and can be between 5 and 188Hz

#define MPU_LPF_RATE   100

//  SERIAL_PORT_SPEED defines the speed to use for the debug serial port

#define  SERIAL_PORT_SPEED  115200

int loopState;                                              // what code to run in the loop

#define  LOOPSTATE_NORMAL  0                                // normal execution
#define  LOOPSTATE_MAGCAL  1                                // mag calibration
#define  LOOPSTATE_ACCELCAL  2                              // accel calibration

static CALLIB_DATA calData;                                 // the calibration data

long lastPollTime;                                          // last time the MPU-9150 was checked
long pollInterval;                                          // gap between polls to avoid thrashing the I2C bus

void magCalStart(void);
void magCalLoop(void);
void accelCalStart(void);
void accelCalLoop(void);

void mpuInit()
{
  dueMPU.selectDevice(DEVICE_TO_USE);                        // only really necessary if using device 1
  dueMPU.init(MPU_UPDATE_RATE, MPU_MAG_MIX_GYRO_AND_MAG, MAG_UPDATE_RATE, MPU_LPF_RATE);   // start the MPU
}

boolean duePoll()
{
  if ((millis() - lastPollTime) < pollInterval)
    return false;                                            // not time yet
  if (dueMPU.read()) {
    lastPollTime = millis();
    return true;
  } 
  return false;                                              // try again next time round
}

void setup()
{
  Serial.begin(SERIAL_PORT_SPEED);
  Serial.print("Arduino9150 starting using device "); Serial.println(DEVICE_TO_USE);
  Wire.begin();
  mpuInit();
  loopState = LOOPSTATE_NORMAL;
  pollInterval = (1000 / MPU_UPDATE_RATE) - 1;              // a bit less than the minimum interval
  lastPollTime = millis();
}

void loop()
{  
  if (Serial.available()) {
    switch (Serial.read()) {
      case 'm':
      case 'M':
        magCalStart();
        return;
        
      case 'a':
      case 'A':
        accelCalStart();
        return;
    }
  }  
  
  dueMPU.selectDevice(DEVICE_TO_USE);                         // only needed if device has changed since init but good form anyway
  switch (loopState) {
    case LOOPSTATE_NORMAL:
      if (duePoll()) {                                        // get the latest data if ready yet
//      dueMPU.printQuaternion(dueMPU.m_rawQuaternion);       // print the raw quaternion from the dmp
//      dueMPU.printVector(dueMPU.m_rawMag);                  // print the raw mag data
//      dueMPU.printVector(dueMPU.m_rawAccel);                // print the raw accel data
//      dueMPU.printAngles(dueMPU.m_dmpEulerPose);            // the Euler angles from the dmp quaternion
//      dueMPU.printVector(dueMPU.m_calAccel);                // print the calibrated accel data
//      dueMPU.printVector(dueMPU.m_calMag);                  // print the calibrated mag data
        dueMPU.printAngles(dueMPU.m_fusedEulerPose);          // print the output of the data fusion
        Serial.println();
      }
      break;
        
    case LOOPSTATE_MAGCAL:
      magCalLoop();
      break;
      
    case LOOPSTATE_ACCELCAL:
      accelCalLoop();
      break;
 }
}

void magCalStart(void)
{
  calLibRead(DEVICE_TO_USE, &calData);                     // pick up existing accel data if there   

  calData.magValid = false;
  calData.magMinX = 0x7fff;                                // init mag cal data
  calData.magMaxX = 0x8000;
  calData.magMinY = 0x7fff;                              
  calData.magMaxY = 0x8000;
  calData.magMinZ = 0x7fff;                             
  calData.magMaxZ = 0x8000; 
 
  Serial.print("\n\nEntering mag calibration mode for device: "); Serial.println(DEVICE_TO_USE); 
  loopState = LOOPSTATE_MAGCAL;
}

void magCalLoop()
{
  boolean changed;
  
  if (duePoll()) {                                         // get the latest data
    changed = false;
    if (dueMPU.m_rawMag[VEC3_X] < calData.magMinX) {
      calData.magMinX = dueMPU.m_rawMag[VEC3_X];
      changed = true;
    }
     if (dueMPU.m_rawMag[VEC3_X] > calData.magMaxX) {
      calData.magMaxX = dueMPU.m_rawMag[VEC3_X];
      changed = true;
    }
    if (dueMPU.m_rawMag[VEC3_Y] < calData.magMinY) {
      calData.magMinY = dueMPU.m_rawMag[VEC3_Y];
      changed = true;
    }
     if (dueMPU.m_rawMag[VEC3_Y] > calData.magMaxY) {
      calData.magMaxY = dueMPU.m_rawMag[VEC3_Y];
      changed = true;
    }
    if (dueMPU.m_rawMag[VEC3_Z] < calData.magMinZ) {
      calData.magMinZ = dueMPU.m_rawMag[VEC3_Z];
      changed = true;
    }
    if (dueMPU.m_rawMag[VEC3_Z] > calData.magMaxZ) {
      calData.magMaxZ = dueMPU.m_rawMag[VEC3_Z];
      changed = true;
    }
 
    if (changed) {
      Serial.println("-------");
      Serial.print("minX: "); Serial.print(calData.magMinX);
      Serial.print(" maxX: "); Serial.print(calData.magMaxX); Serial.println();
      Serial.print("minY: "); Serial.print(calData.magMinY);
      Serial.print(" maxY: "); Serial.print(calData.magMaxY); Serial.println();
      Serial.print("minZ: "); Serial.print(calData.magMinZ);
      Serial.print(" maxZ: "); Serial.print(calData.magMaxZ); Serial.println();
    }
  }
  
  if (Serial.available()) {
    switch (Serial.read()) {
      case 's':
      case 'S':
        calData.magValid = true;
        calLibWrite(DEVICE_TO_USE, &calData);
        Serial.print("Mag cal data saved for device "); Serial.println(DEVICE_TO_USE);
        break;
        
      case 'x':
      case 'X':
        loopState = LOOPSTATE_NORMAL;
        Serial.println("\n\n *** restart to use calibrated data ***");
        break;
    }
  }  
}

void accelCalStart(void)
{
  calLibRead(DEVICE_TO_USE, &calData);                     // pick up existing accel data if there   

  calData.accelValid = false;
  calData.accelMinX = 0x7fff;                              // init accel cal data
  calData.accelMaxX = 0x8000;
  calData.accelMinY = 0x7fff;                              
  calData.accelMaxY = 0x8000;
  calData.accelMinZ = 0x7fff;                             
  calData.accelMaxZ = 0x8000;
 
  Serial.print("\n\nEntering accel calibration mode for device: "); Serial.println(DEVICE_TO_USE); 
  loopState = LOOPSTATE_ACCELCAL;
  dueMPU.disableAccelCal();
}

void accelCalLoop()
{
  boolean changed;
  
  if (duePoll()) {                                          // get the latest data
    changed = false;
    if (dueMPU.m_rawAccel[VEC3_X] < calData.accelMinX) {
      calData.accelMinX = dueMPU.m_rawAccel[VEC3_X];
      changed = true;
    }
    if (dueMPU.m_rawAccel[VEC3_X] > calData.accelMaxX) {
      calData.accelMaxX = dueMPU.m_rawAccel[VEC3_X];
      changed = true;
    }
    if (dueMPU.m_rawAccel[VEC3_Y] < calData.accelMinY) {
      calData.accelMinY = dueMPU.m_rawAccel[VEC3_Y];
      changed = true;
    }
    if (dueMPU.m_rawAccel[VEC3_Y] > calData.accelMaxY) {
      calData.accelMaxY = dueMPU.m_rawAccel[VEC3_Y];
      changed = true;
    }
    if (dueMPU.m_rawAccel[VEC3_Z] < calData.accelMinZ) {
      calData.accelMinZ = dueMPU.m_rawAccel[VEC3_Z];
      changed = true;
    }
    if (dueMPU.m_rawAccel[VEC3_Z] > calData.accelMaxZ) {
      calData.accelMaxZ = dueMPU.m_rawAccel[VEC3_Z];
      changed = true;
    }
 
    if (changed) {
      Serial.println("-------");
      Serial.print("minX: "); Serial.print(calData.accelMinX);
      Serial.print(" maxX: "); Serial.print(calData.accelMaxX); Serial.println();
      Serial.print("minY: "); Serial.print(calData.accelMinY);
      Serial.print(" maxY: "); Serial.print(calData.accelMaxY); Serial.println();
      Serial.print("minZ: "); Serial.print(calData.accelMinZ);
      Serial.print(" maxZ: "); Serial.print(calData.accelMaxZ); Serial.println();
    }
  }
  
  if (Serial.available()) {
    switch (Serial.read()) {
      case 's':
      case 'S':
        calData.accelValid = true;
        calLibWrite(DEVICE_TO_USE, &calData);
        Serial.print("Accel cal data saved for device "); Serial.println(DEVICE_TO_USE);
        break;
        
      case 'x':
      case 'X':
        loopState = LOOPSTATE_NORMAL;
        Serial.println("\n\n *** restart to use calibrated data ***");
       break;
    }
  }  
}

