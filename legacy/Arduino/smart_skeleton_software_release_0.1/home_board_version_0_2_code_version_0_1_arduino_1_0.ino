//smart_skeleton_home_board code version 0.1 by John M. Pattillo 

//This code is written for an arduino fitted the smart skeleton home board v0.2-v0.3.
//The code allows communication of the home board with smartskeleton IMU v 0.25 and later by John Pattillo (www.jpattillo.net).
//The code is based in part on "FreeIMU_quaternion.pde" distributed as an example with the FreeIMU library by Fabio Varesano (www.varesano.net),
//and also on the examples included with the "Wire" library by Nicholas Zambetti (www.zambetti.com).

//This code has been tested with arduino IDE v1.0

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

#define load_switch A0

#include <Wire.h>

int IMUaddress[] = {4,5,1,0}; //An array of the i2c addresses of the IMUs.  TODO in future this will be built automatically at startup.
int IMUindex = 0;  //this tracks the index of the IMUaddress array for switching communication to different IMUs
int maxindex = 3; // maximum index of the IMUaddress array needed for wrapping the array - this will also be determined automatically in the future


//Since each IMU has it's own ATMEGA chip, one IMU can be acquiring quaternions while another is sending previously acquired quaternions to the home board

int IMU; // The imu acquiring quaternions
int IMU2; //the imu sending quaternions to the home board

 

int sent = 0;
int received = 0;
int bytecounter = 0;
int beginning = 1;
boolean led_switch = true;


int homeBoardState = 0; //both the home board and IMU code use a case switch statement to switch between different modes - this is the initial state.

void setup()
{
  Wire.begin(20); // join i2c bus as address 20
  Wire.onReceive(receiveEvent); //calls function to run upon recieving data
  Serial.begin(115200); //For sending quaternions to computer.  
  
  //status LED control
  //A2 HIGH and A3 LOW = amber LED
  //A2 LOW and A3 HIGH = green LED  
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  
  digitalWrite(A2, HIGH); digitalWrite(A3, LOW); //turn on amber LED during startup
   
  pinMode(load_switch, OUTPUT); //load switch control: 5V line on cat5 cable turned off when low
  digitalWrite(load_switch, LOW); // turn off IMU bus power
  delay(100); //let IMUs power down)
  digitalWrite(load_switch, HIGH); //turn on IMU bus power 
  delay(5000); //give the IMUs time to boot
  digitalWrite(A2, LOW); digitalWrite(A3, HIGH); //turn on green LED to indicate finished booting - TODO, set up an IMU diagnostic to run here and don't turn on green unless they all pass
}


void loop()
{ 

  //this block of code is for the use of a button to start things off - this is just for development purposes
 /* if ( beginning==1){ 
  int reading = digitalRead(8);
  if (reading==LOW) {beginning = 0;digitalWrite(A3, LOW);delay(100);digitalWrite(A3,HIGH);}
   } 
  
 if (beginning == 0) { //the following code is executed after the button is pressed */
 
  if(IMUindex == 0) {led_switch = !led_switch; digitalWrite(A3, led_switch);digitalWrite(A2, !led_switch);}
  switch(homeBoardState) { 
  
    case 0: { //need to command an IMU to acquire quarternions - this is the case at the beginning
      IMU = IMUaddress[IMUindex]; //set the IMU address to command an IMU to acquire quaternions
     // digitalWrite(A3, HIGH); //turn on green LED
      Wire.beginTransmission(IMU); //set up transmission to IMU 
      Wire.write("G"); //que up request for quaternions "G" for "Get Quaternions"
      Wire.endTransmission(); //send request
      delayMicroseconds(50); //IMU code will disable PCA9509 chip, give it some time to do this.
      homeBoardState = 2;
      break;
    }
    
    /*  case 1: { //waiting to receive "R"eady signal from IMU - may not need this case
      digitalWrite(11,LOW);
      break;
    } */
    
     
    case 2: { //tell the IMU that acquired quaternions in the previous loop to transmit its quarternions.
      //digitalWrite(A3, LOW); //turn off green LED
      
      int lastindex = IMUindex - 1;
      if (lastindex < 0) {lastindex = maxindex;} //deal with first index wrap
      IMU2 = IMUaddress[lastindex]; //set address of IMU to request sending of quaternions
      
      //set up quaternion output to computer.  Beginning of the output line indicates the i2c address of the IMU that is sending quaternions
      Serial.print(IMU2); //print the IMU ID number at the beginning of the line
      Serial.print(","); //seperate with a comma - may change this to a colon or something
      
      //send request for quaternions
      Wire.requestFrom(IMU2, 32);    // request 32 bytes from IMU
      bytecounter = 0; //this tracks the number of bytes written to the serial port so that I know when to write commas -change this to local variable?
       while(Wire.available())    
          { 
              char c = Wire.read(); // receive a byte as character
              bytecounter++;
              Serial.print(c);         // print the character
              if (bytecounter >7) {int tester = bytecounter%8; if(tester==0) {Serial.print(",");}}//use modulo to determine if number of bytes recieved is a multiple of 8 and we would need a comma here
              
         }
       // digitalWrite(A3, HIGH); 
        Serial.println(""); //line feed     
        homeBoardState = 3;
       // digitalWrite(A3, LOW); 
        break;
    } 
    
  
    case 3: {  //Poll the IMU that has been acquring quaternions to see if it is ready to transmit them.
    //sending the T expect an R in return - TODO:  put a millis trick in here to generate a switch to error state if the IMU doesn't respond 
      
      if (sent == 0) { //send the T to request
      delay(4);
     // digitalWrite(A3, HIGH); //turn on green LED
      Wire.beginTransmission(IMU);
      Wire.write("T"); //send a T to the IMU to see if it is ready to transmit the quaternions
      Wire.endTransmission();
      sent = 1;
      }
      if (received == 1) { //got an "R" for "Ready" back from the IMU
        homeBoardState = 0; //will go back to the first state
        IMUindex++; //move to the next IMU
        if (IMUindex > maxindex) {IMUindex = 0;} //deal with last index wrap
        sent = 0;
        received = 0;
       // digitalWrite(A3, LOW); //turn off green LED     
      }            
      break;      
    }  //end case 3
    
    
    case 4: { //error state
      for (int x = 1; x<=4; x++) {
      digitalWrite(A2, HIGH); digitalWrite(A3, LOW); //turn off green LED and turn on amber LED
      delay(100);
      digitalWrite(A2, LOW); //turn off amber LED
      delay(100);
      }
      break;
    } //end case 4 
      
 
  } //end case switch
  
//  } //end if for beginning with button press
} //end void loop

void receiveEvent (int howMany)
 {
 switch (homeBoardState) {
  case 0: {
   Serial.println("error in case 0"); //shouldn't get any data in this state
   delay(1);
   homeBoardState = 4;
   break;
   }  
 
 
   case 3: { //recieve responses from IMU
   while(1 < Wire.available()) 
    {
    char c = Wire.read(); // receive byte as a character
    }
   char dothis = Wire.read();
   digitalWrite(11, LOW);
   if (dothis == 'R') {
    received = 1;
   } 
   else {homeBoardState = 4;}  //error state - should only recieve "R" in this state 
   break; 
   }
 
   
   case 4: { //in error state - do nothing 
    delayMicroseconds(1);
    break;
   }
 } //end switch case
 } //end receiveEvent
