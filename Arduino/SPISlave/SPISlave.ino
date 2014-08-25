// SPI slave for MPU-9150

// SPI code written by by Nick Gammon, February 2011 -- modified by BIT BLUEPRINT for ROKOKO.co
 
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
#define MPU_UPDATE_RATE  (20)
#define MAG_UPDATE_RATE  (10)
#define MPU_MAG_MIX_GYRO_ONLY          0                  // just use gyro yaw
#define MPU_MAG_MIX_MAG_ONLY           1                  // just use magnetometer and no gyro yaw
#define MPU_MAG_MIX_GYRO_AND_MAG       10                 // a good mix value 
#define MPU_MAG_MIX_GYRO_AND_SOME_MAG  50                 // mainly gyros with a bit of mag correction 
#define MPU_LPF_RATE   5
#define SERIAL_PORT_SPEED  57600
MPUQuaternion gravity;                                     // this is our earth frame gravity vector             
#include <SPI.h>

char buf [100];
char* output = "abcdefghijklmn";

volatile byte pos;
volatile boolean process_it;
 
void setup (void)
{
  Serial.begin (115200);   // debugging
 
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  pinMode(SS, INPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;
 
  // now turn on interrupts
  SPI.attachInterrupt();
 
  // Reset the MPU-9150
  Wire.begin();
  MPU.init(MPU_UPDATE_RATE, MPU_MAG_MIX_GYRO_AND_MAG, MAG_UPDATE_RATE, MPU_LPF_RATE);     // start the MPU

  gravity[QUAT_W] = 0;
  gravity[QUAT_X] = 0;
  gravity[QUAT_Y] = 0;
  gravity[QUAT_Z] = SENSOR_RANGE;
  
  delay(5); 
 
}  // end of setup
 
 
// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;  // grab byte from SPI Data Register
  
  SPDR = output[(pos++) % 16];
}


// main loop - wait for flag set in interrupt routine
void loop (void)
{
  MPUQuaternion rotatedGravity;                            // this is our body frame gravity vector
  MPUQuaternion fusedConjugate;                            // this is the conjugate of the fused quaternion
  MPUQuaternion qTemp;                                     // used in the rotation
  MPUVector3 result;                                       // the accelerations
  
  if (MPU.read()) {                                        // get the latest data
  
    MPUQuaternionConjugate(MPU.m_fusedQuaternion, fusedConjugate);  // need this for the rotation
    
    //  rotate the gravity vector into the body frame
    MPUQuaternionMultiply(gravity, MPU.m_fusedQuaternion, qTemp);
    MPUQuaternionMultiply(fusedConjugate, qTemp, rotatedGravity);
    
    //  now subtract rotated gravity from the body accels to get real accelerations
    //  note that signs are reversed to get +ve acceleration results in the conventional axes.
    result[VEC3_X] = -(MPU.m_calAccel[VEC3_X] - rotatedGravity[QUAT_X]);
    result[VEC3_Y] = -(MPU.m_calAccel[VEC3_Y] - rotatedGravity[QUAT_Y]);
    result[VEC3_Z] = -(MPU.m_calAccel[VEC3_Z] - rotatedGravity[QUAT_Z]);

    
    
    /*
    //  ## these variables are the values from the MPU ## //
    Serial.print("DS");
    Serial.print("\n");
 
    // the fused quaternion
    Serial.print(MPU.m_fusedQuaternion[QUAT_W]);Serial.print("!");
    Serial.print(MPU.m_fusedQuaternion[QUAT_X]);Serial.print("!");  
    Serial.print(MPU.m_fusedQuaternion[QUAT_Y]);Serial.print("!");
    Serial.print(MPU.m_fusedQuaternion[QUAT_Z]);
    
    Serial.print("\n");
    
    // the residual accelerations 
    Serial.print(result[VEC3_X]);Serial.print("!");
    Serial.print(result[VEC3_Y]);Serial.print("!");
    Serial.print(result[VEC3_Z]);
    
    // This should be integrated twice to the get the position
    // http://www.varesano.net/blog/fabio/simple-gravity-compensation-9-dom-imus
    
    Serial.print("&");
    */
  }
}
