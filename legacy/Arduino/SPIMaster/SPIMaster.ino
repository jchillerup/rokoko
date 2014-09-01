// Written by Nick Gammon
// April 2011


#include <SPI.h>
#include "pins_arduino.h"

void setup (void)
{
  Serial.begin (115200);
  Serial.println ();
  
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SS, OUTPUT);
  
  digitalWrite(SS, HIGH);  // ensure SS stays high for now

  // Put SCK, MOSI, SS pins into output mode
  // also put SCK, MOSI into LOW state, and SS into HIGH state.
  // Then put SPI hardware into Master mode and turn SPI on
  SPI.begin ();

  // Slow down the master a bit
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  
}

byte transferAndWait (const byte what)
{
  byte a = SPI.transfer(what);
  
  return a;
} 

void loop (void)
{
  byte request, reply;
  
  // enable Slave Select
  digitalWrite(SS, LOW);    

  float reading[4];

  for (int i = 0; i < 16; i++) {
    reply = transferAndWait (i);
    /*Serial.print(i);
    Serial.print(": ");
    Serial.println(reply);*/
    ((byte*) reading)[(i-1)%16] = reply;
  }
  Serial.print(reading[0]);
  Serial.print(" ");
  Serial.print(reading[1]);
  Serial.print(" ");
  Serial.print(reading[2]);
  Serial.print(" ");
  Serial.println(reading[3]);
}

