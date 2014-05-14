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


#define NOP 0
#define CALIB_ACCEL 1
#define CALIB_MAG 2
#define RUNNING 3


MPUQuaternion gravity;                                     // this is our earth frame gravity vector
char LOOPSTATE = NOP;
char LEDSTATE = LOW;

CALLIB_DATA calData;
long endAtTime;  // Store the timestamp for when the calibrations should end.

void setup()
{
  Serial.begin(SERIAL_PORT_SPEED);
  Serial.println("Accel9150 starting");
  Wire.begin();
  calLibRead(0, &calData);
  
  MPU.selectDevice(0);
  MPU.useAccelCal(true);
  MPU.useMagCal(true);
 
  MPU.init(MPU_UPDATE_RATE, MPU_MAG_MIX_GYRO_AND_MAG, MAG_UPDATE_RATE, MPU_LPF_RATE);     // start the MPU
  
  //  set up the initial gravity vector for quaternion rotation - max value down the z axis

  gravity[QUAT_W] = 0;
  gravity[QUAT_X] = 0;
  gravity[QUAT_Y] = 0;
  gravity[QUAT_Z] = SENSOR_RANGE;

  pinMode(13, OUTPUT);
  
  delay(5);
}


// PROCEDURE FOR CALIBRATING THE ACCELEROMETER, STARTUP
void accelCalStart() {
  calLibRead(0, &calData);                                    // pick up existing accel data if there   

  calData.accelValid = false;
  calData.accelMinX = 0x7fff;                                // init accel cal data
  calData.accelMaxX = 0x8000;
  calData.accelMinY = 0x7fff;                              
  calData.accelMaxY = 0x8000;
  calData.accelMinZ = 0x7fff;                             
  calData.accelMaxZ = 0x8000;
  
  MPU.useAccelCal(false);                                  // disable accel offsets
}

// PROCEDURE FOR CALIBRATING THE ACCELEROMETER, LOOP
void accelCalLoop() {
  boolean changed;
  if (MPU.read()) {                                        // get the latest data
    changed = false;
    if (MPU.m_rawAccel[VEC3_X] < calData.accelMinX) {
      calData.accelMinX = MPU.m_rawAccel[VEC3_X];
      changed = true;
    }
    if (MPU.m_rawAccel[VEC3_X] > calData.accelMaxX) {
      calData.accelMaxX = MPU.m_rawAccel[VEC3_X];
      changed = true;
    }
    if (MPU.m_rawAccel[VEC3_Y] < calData.accelMinY) {
      calData.accelMinY = MPU.m_rawAccel[VEC3_Y];
      changed = true;
    }
    if (MPU.m_rawAccel[VEC3_Y] > calData.accelMaxY) {
      calData.accelMaxY = MPU.m_rawAccel[VEC3_Y];
      changed = true;
    }
    if (MPU.m_rawAccel[VEC3_Z] < calData.accelMinZ) {
      calData.accelMinZ = MPU.m_rawAccel[VEC3_Z];
      changed = true;
    }
    if (MPU.m_rawAccel[VEC3_Z] > calData.accelMaxZ) {
      calData.accelMaxZ = MPU.m_rawAccel[VEC3_Z];
      changed = true;
    }
  }
}

// PROCEDURE FOR CALIBRATING THE ACCELEROMETER, STORAGE
void accelCalSave() {
  calData.accelValid = true;
  calLibWrite(0, &calData);
  Serial.println("Accel calibration data saved");  
}



// PROCEDURE FOR CALIBRATING THE MAGNETOMETER, STARTUP
void magCalStart() {
  calLibRead(0, &calData);                                    // pick up existing accel data if there   

  calData.magValid = false;
  calData.magMinX = 0x7fff;                                // init mag cal data
  calData.magMaxX = 0x8000;
  calData.magMinY = 0x7fff;                              
  calData.magMaxY = 0x8000;
  calData.magMinZ = 0x7fff;                             
  calData.magMaxZ = 0x8000;
}


// PROCEDURE FOR CALIBRATING THE MAGNETOMETER, LOOP
void magCalLoop() {
  boolean changed;
  if (MPU.read()) {                                        // get the latest data
    changed = false;
    if (MPU.m_rawMag[VEC3_X] < calData.magMinX) {
      calData.magMinX = MPU.m_rawMag[VEC3_X];
      changed = true;
    }
     if (MPU.m_rawMag[VEC3_X] > calData.magMaxX) {
      calData.magMaxX = MPU.m_rawMag[VEC3_X];
      changed = true;
    }
    if (MPU.m_rawMag[VEC3_Y] < calData.magMinY) {
      calData.magMinY = MPU.m_rawMag[VEC3_Y];
      changed = true;
    }
     if (MPU.m_rawMag[VEC3_Y] > calData.magMaxY) {
      calData.magMaxY = MPU.m_rawMag[VEC3_Y];
      changed = true;
    }
    if (MPU.m_rawMag[VEC3_Z] < calData.magMinZ) {
      calData.magMinZ = MPU.m_rawMag[VEC3_Z];
      changed = true;
    }
     if (MPU.m_rawMag[VEC3_Z] > calData.magMaxZ) {
      calData.magMaxZ = MPU.m_rawMag[VEC3_Z];
      changed = true;
    }
  }
}

// PROCEDURE FOR CALIBRATING THE MAGNETOMETER, STORAGE
void magCalSave() {
  calData.magValid = true;
  calLibWrite(0, &calData);
  Serial.println("Mag cal data saved");
}


void loop()
{
  if (Serial.available()) {
    switch (Serial.read()) {
      case 'a':
      case 'A':
        if (LOOPSTATE == CALIB_MAG || LOOPSTATE == CALIB_ACCEL) { break; }
        
        LOOPSTATE = CALIB_ACCEL;
        accelCalStart();
        endAtTime = millis() + 10*1000;
        
        while(millis() < endAtTime) {
          accelCalLoop(); 
        }
        
        accelCalSave();
        Serial.println("Calibrated the accelerometer");  
  
        LOOPSTATE = NOP;      
        break;

      case 'm':
      case 'M':
        if (LOOPSTATE == CALIB_MAG || LOOPSTATE == CALIB_ACCEL) { break; }
        
        LOOPSTATE = CALIB_MAG;
        magCalStart();
        endAtTime = millis() + 10*1000;
        
        while(millis() < endAtTime) {
          magCalLoop(); 
        }
        
        magCalSave();
        Serial.println("Calibrated the magnetometer");
        
        LOOPSTATE = NOP;
        break;
        
      case 'g':
      case 'G':
        LOOPSTATE = RUNNING;
        break;
        
      case 'l':
      case 'L':
        LEDSTATE = LEDSTATE==HIGH?LOW:HIGH;
        digitalWrite(13, LEDSTATE);
        break;
    }
  }
  
  
  switch (LOOPSTATE) {
    case NOP: 
    break;
    
    case RUNNING:
    default:  
      MPUQuaternion rotatedGravity;                            // this is our body frame gravity vector
      MPUQuaternion fusedConjugate;                            // this is the conjugate of the fused quaternion
      MPUQuaternion qTemp;                                     // used in the rotation
    
      if (MPU.read()) {                                        // get the latest data
        MPUQuaternionConjugate(MPU.m_fusedQuaternion, fusedConjugate);  // need this for the rotation
    
        //  rotate the gravity vector into the body frame
        MPUQuaternionMultiply(gravity, MPU.m_fusedQuaternion, qTemp);
        MPUQuaternionMultiply(fusedConjugate, qTemp, rotatedGravity);
    
        //  ## these variables are the values from the MPU ## //
        Serial.write("DS");
        Serial.write("\n");
    
        // the fused quaternion
        Serial.print(MPU.m_fusedQuaternion[QUAT_W]); Serial.write("!");
        Serial.print(MPU.m_fusedQuaternion[QUAT_X]); Serial.write("!");
        Serial.print(MPU.m_fusedQuaternion[QUAT_Y]); Serial.write("!");
        Serial.print(MPU.m_fusedQuaternion[QUAT_Z]);
    
        Serial.print("\n");
    
        // the residual accelerations
    
        //  now subtract rotated gravity from the body accels to get real accelerations
        //  note that signs are reversed to get +ve acceleration results in the conventional axes.
        Serial.print(-(MPU.m_calAccel[VEC3_X] - rotatedGravity[QUAT_X])); Serial.write("!");
        Serial.print(-(MPU.m_calAccel[VEC3_Y] - rotatedGravity[QUAT_Y])); Serial.write("!");
        Serial.print(-(MPU.m_calAccel[VEC3_Z] - rotatedGravity[QUAT_Z]));
    
        // This should be integrated twice to the get the position
        // http://www.varesano.net/blog/fabio/simple-gravity-compensation-9-dom-imus
    
        Serial.write("&");
      }
    break;
  }
}

