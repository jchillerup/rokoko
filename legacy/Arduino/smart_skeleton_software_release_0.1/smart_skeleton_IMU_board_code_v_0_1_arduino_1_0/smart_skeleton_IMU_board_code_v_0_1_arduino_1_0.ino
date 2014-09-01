//smart_skeleton_IMU code version 0.1 by John M. Pattillo 

//This code is written for the smartskeleton IMU v 0.25 and later by John Pattillo (www.jpattillo.net)
//The code is based in part on "FreeIMU_quaternion.pde" distributed as an example with the FreeIMU library by Fabio Varesano (www.varesano.net),
//and also on the examples included with the "Wire" library by Nicholas Zambetti (www.zambetti.com).
//The required libraries, FreeIMU, ADXL345, HMC58X3, and ITG3200 are by Fabio Varesano and available for download at varesano.net.
//Files modified from varesano.net, FreeIMU_ss.h and FreeIMU_ss.cpp should be included with this file and should be placed in the FreeIMU library folder

/*   This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

//

#include <ADXL345.h> 
#include <HMC58X3.h> 
#include <ITG3200.h>
#include <I2Cdev.h>
#include <MPU6050.h>

#define DEBUG
#ifdef DEBUG
#include "DebugUtils.h"
#endif

#include "CommunicationUtils.h"
#include "FreeIMU_ss.h"

#include <Wire.h>


//TODO store the following 3 values in EEPROM using a seperate sketch.  This will help keep the IMU addresses consistent when the rest of the code is upgraded
#define home_board_address 20 //address of the home arduino
#define enablepin 3 //this is either 3 or 4 depending on the version of the board.  in the most current version at the time of this writing (0.5) it is 4
#define IMUaddress 4 //address of this IMU - Planned for addresses 1-19 to be open for IMUs.  I can't see needing more than that for my purposes

float q[4]; //this array will hold the quaternions

FreeIMU my3IMU = FreeIMU(); //instantiate IMU as my3IMU

int IMUstate = 0; //this variable will be adjusted depending on commands from the home board. 0 means do nothing
char sendstring[] = "00000000000000000000000000000000"; //this char array stores the quaternions to be transmitted. 
//Sends all zeros if it is first IMU to be polled and hasn't calculated quaternions

void setup()
 {
  pinMode(enablepin, OUTPUT); digitalWrite(enablepin, LOW);  //PCA9509 enable pin HIGH = enabled, LOW = disabled, do not access sensors unless disabled.  
  //Start off with disaabled for accessing sensors during initiation of FreeIMU library.
  
  pinMode(A2, OUTPUT); digitalWrite(A2, LOW);  //amber LED
  pinMode(A3, OUTPUT); digitalWrite(A3, HIGH);  //turn on green LED to indicate power
  
  Wire.begin(IMUaddress);       // join i2c bus with address IMUaddress 
  Wire.onReceive(receiveEvent); // register event - what happens during the receiveEvent will depend on the IMUstate, when getting the quaternions, it does nothing
  Wire.onRequest(requestEvent); // register request event - the home IMU board will request the quaternions.
  delay(5);
  my3IMU.init(); //initiate the FreeIMU library
  delay(5);
  digitalWrite(enablepin, HIGH); //enable PCA9509 so that you listen to the main i2c bus
 } //end void setup

void loop()
{
 switch(IMUstate) {
  
  case 0: {delayMicroseconds(1);break;} //do nothing wait for G message from home board
    
  case 1: {  //acquire quaternions
    digitalWrite(A2, HIGH); //turn on amber LED to indicate sensor access
    digitalWrite(enablepin, LOW); //disable PCA9509 access to main bus
    delayMicroseconds(10); //give the bus a little time to turn off
    my3IMU.getQ(q); //acquire quaternions
    
    constructFloatArray(q,4,sendstring,32); // load the sendstring array with the quaternions.
    
    digitalWrite(enablepin, HIGH); //enable PCA9509 access to main bus
    digitalWrite(A2, LOW); //turn off amber LED   
    delayMicroseconds(5); //allow bus to settle - not sure this is necessary
    IMUstate = 2;
    break;   
    } //end case 1
  
  case 2: {delayMicroseconds(1);break;} //do nothing and wait for "T" message from home board
     

 
  case 3: {  // send an R for "R"eady to be accessed- this is only accessed if the IMU has received "T" from home board: see receiveEvent
   Wire.beginTransmission(home_board_address);
   Wire.write("R");
   Wire.endTransmission();
   IMUstate = 0; //go back to intitial state
   break; 
   } //end case 3 
  
  case 4: { //error state - just flash the LED a few times - TODO:  have the IMU send some sort of indication to the home board that it is in the error state.
    for (int x = 1; x<=4; x++) {
     digitalWrite(A3, LOW); //turn off green LED 
     digitalWrite(A2, HIGH);
     delay(100);
     digitalWrite(A2, LOW);
     delay(100);
     }
    break;
    } //end case 4 
  } //end switch case
} //end void loop


//receiveEvent is accessed whenever data is received
void receiveEvent(int howMany)
 { 
 switch (IMUstate) {   
  case 0: {
  while(1 < Wire.available()) // loop through all but the last
    {
     char c = Wire.read(); // receive byte as a character
    }
    char dothis = Wire.read();    // receive byte as an integer
    if (dothis == 'G') {IMUstate = 1;}
    else {IMUstate = 4;} //error state, should only recieve a G in ths state
    break;
   } //end case 0
 
  case 1:    {
     while(1 <= Wire.available()) // loop through all but the last
     {
     char c = Wire.read(); // receive byte as a character
     } 
   IMUstate = 4; //there shouldn't be any input during this state
   break;
   } //end case 1
 
  case 2: {
  while(1 < Wire.available()) // loop through all but the last
    {
     char c = Wire.read(); // receive byte as a character
    }
    char dothis = Wire.read();    // receive byte as an integer
    if (dothis == 'T') {IMUstate = 3;} //switch to state to send R
    else {IMUstate = 4;} //error state
    break;
   } //end case 2
 
  
  case 4:  {
     while(1 <= Wire.available()) // loop through all but the last
    {
     char c = Wire.read(); // receive byte as a character
    } 
    delayMicroseconds(1);//do nothing - in error state
    break;
   } //end case 4 
  } //end switch state
 } // end receiveEvent
 
 void requestEvent()
{
  Wire.write(sendstring); // send the quaternions (32 bytes) 
 
}


  //The following code adapted from the Communication_Util library distributed with the FreeIMU
  //library by Fabio Varesano (www.varesano.net).
void constructFloatArray(float * arr, int length, char * chararray, int charlength) {
  int placetracker = 0; 
  for(int i=0; i<length; i++) {
    float f = arr[i];
  
     byte * b = (byte *) &f;

    for(int i=0; i<4; i++) { 
      byte b1 = (b[i] >> 4) & 0x0f;
      byte b2 = (b[i] & 0x0f);
    
      char c1 = (b1 < 10) ? ('0' + b1) : 'A' + b1 - 10;
      char c2 = (b2 < 10) ? ('0' + b2) : 'A' + b2 - 10;
    
      chararray[placetracker] = c1;
      placetracker++;
      chararray[placetracker] = c2;
      if(placetracker != (charlength-1)) {placetracker++;}
    } //end nested for 
    
    
  } //end for


}

