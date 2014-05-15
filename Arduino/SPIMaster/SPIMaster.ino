// Written by Nick Gammon
// April 2011


#include <SPI.h>
#include "pins_arduino.h"

void setup (void)
{
  Serial.begin (115200);
  Serial.println ();
  
  digitalWrite(SS, HIGH);  // ensure SS stays high for now

  // Put SCK, MOSI, SS pins into output mode
  // also put SCK, MOSI into LOW state, and SS into HIGH state.
  // Then put SPI hardware into Master mode and turn SPI on
  SPI.begin ();

  // Slow down the master a bit
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  
}  // end of setup

byte transferAndWait (const byte what)
{
  byte a = SPI.transfer (what);
  delayMicroseconds (200);
  return a;
} // end of transferAndWait

void loop (void)
{

  byte request, reply;
  
  // enable Slave Select
  digitalWrite(SS, LOW);    

  request = 'a';
  reply = transferAndWait (request);  // add command

  Serial.print("Sent: ");
  Serial.print(request);
  Serial.print(", received: ");
  Serial.println(reply);
  
  delay (10);  // 1 second delay 
}  // end of loop
