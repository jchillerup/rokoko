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

#define I2C_SPI_TOGGLE

#include <Wire.h>
#include "I2Cdev.h"
#include "MPU9150Lib.h"
#include "CalLib.h"
#include <dmpKey.h>
#include <dmpmap.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>
#include <EEPROM.h>
#include <SPI.h>
#include "pins_arduino.h"

MPU9150Lib MPU;                                              // the MPU object

//  MPU_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the sensor data and DMP output

#define MPU_UPDATE_RATE  (25)

//  MAG_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the magnetometer data
//  MAG_UPDATE_RATE should be less than or equal to the MPU_UPDATE_RATE

#define MAG_UPDATE_RATE  (25)

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
#define  SERIAL_PORT_SPEED  9600

#define NOP 0
#define CALIB_ACCEL 1
#define CALIB_MAG 2
#define INIT 3

MPUQuaternion gravity;                                     // this is our earth frame gravity vector
char LOOPSTATE = NOP;
char LEDSTATE = LOW;

MPUQuaternion rotatedGravity;                            // this is our body frame gravity vector
MPUQuaternion fusedConjugate;                            // this is the conjugate of the fused quaternion
MPUQuaternion qTemp;                                     // used in the rotation


CALLIB_DATA calData;
long endAtTime;  // Store the timestamp for when the calibrations should end.
char * IDENTIFIER = (char*) calloc(16, sizeof(byte));

void setup()
{
  int ident_eeprom_address = sizeof(CALLIB_DATA);

  Serial.begin(SERIAL_PORT_SPEED);
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
  pinMode(4, OUTPUT);

  // Reading the ident from EEPROM
  byte *ptr = (byte *) IDENTIFIER;
  for (byte i = 0; i < 16; i++)
    *ptr++ = EEPROM.read(ident_eeprom_address + i);

  delay(5);

#ifdef I2C_SPI_TOGGLE
  // Prepare the SPI
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  pinMode(SS, INPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // now turn on interrupts
  //  SPCR |= _BV(SPIE);
  SPI.attachInterrupt();
#else
  // Prepare the I2C
  Wire.begin(1);
  Wire.onRequest(i2c_handler);
#endif

  LOOPSTATE = INIT;
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
  Serial.println("AS");
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
  Serial.println("MS");
}


void loop()
{
  char c;
  if (Serial.available()) {
    switch (c = Serial.read()) {
      case 'a':
      case 'A':
        if (LOOPSTATE == CALIB_MAG || LOOPSTATE == CALIB_ACCEL) {
          break;
        }

        Serial.println("ca");

        LOOPSTATE = CALIB_ACCEL;
        accelCalStart();
        endAtTime = millis() + 10 * 1000;

        while (millis() < endAtTime) {
          accelCalLoop();
        }

        accelCalSave();
        Serial.println("CA");

        LOOPSTATE = NOP;
        break;

      case 'm':
      case 'M':
        if (LOOPSTATE == CALIB_MAG || LOOPSTATE == CALIB_ACCEL) {
          break;
        }

        Serial.println("cm");

        LOOPSTATE = CALIB_MAG;
        magCalStart();
        endAtTime = millis() + 10 * 1000;

        while (millis() < endAtTime) {
          magCalLoop();
        }

        magCalSave();
        Serial.println("CM");

        LOOPSTATE = NOP;
        break;

      case 'g':
      case 'G':
        // We need to check for this to avoid sending data while at the same time calibrating.
        if (LOOPSTATE == NOP) {
          Serial.write((byte*) MPU.m_fusedQuaternion, 4 * sizeof(float));
        }
        break;
      case 'j':
      case 'J':
        if (LOOPSTATE == NOP) {
          Serial.write((byte*) MPU.m_fusedQuaternion, 4 * sizeof(float));
          Serial.write((byte*) MPU.m_rawGyro, 3 * sizeof(short));
          Serial.write((byte*) MPU.m_rawAccel, 3 * sizeof(short));
          Serial.write((byte*) MPU.m_rawMag, 3 * sizeof(short));
        }
        break;

      case 'l':
      case 'L':
        LEDSTATE = LEDSTATE == HIGH ? LOW : HIGH;
        digitalWrite(13, LEDSTATE);
        break;

      case 't':
      case 'T':
        Serial.print("IMU             ");
        break;

      case 'i':
        Serial.write(IDENTIFIER, 16);
        break;

      case 'I':
        Serial.readBytes(IDENTIFIER, 16);
        // Put the identifier in the first 16 bytes after the calibration data
        int eeprom_address = sizeof(CALLIB_DATA);

        char* ptr = IDENTIFIER;

        for (byte i = 0; i < 16; i++)
          EEPROM.write(eeprom_address + i, *ptr++);

        break;
    }
  }

  if (MPU.read()) {
    MPUQuaternionConjugate(MPU.m_fusedQuaternion, fusedConjugate);

    //  rotate the gravity vector into the body frame
    MPUQuaternionMultiply(gravity, MPU.m_fusedQuaternion, qTemp);
    MPUQuaternionMultiply(fusedConjugate, qTemp, rotatedGravity);

    // Allow for polling of data
    LOOPSTATE = NOP;
  }
}


#ifdef I2C_SPI_TOGGLE
// SPI interrupt routine
ISR (SPI_STC_vect)
{
  // We're getting something on the SPI bus. Take note of the time so we can timeout
  // if we don't get queried for more bytes in a timely fashion.
  byte c = SPDR;  // grab byte from SPI Data Register
  long start = millis();

  // For now we're just sending the byte that gets queried on the SPI interface.
  SPDR = ((byte*) MPU.m_fusedQuaternion)[c];
}
#else
void i2c_handler() {
  Wire.write((char*) MPU.m_fusedQuaternion);
}
#endif

