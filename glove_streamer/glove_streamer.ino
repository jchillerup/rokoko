#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <EEPROM.h>

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_1X);


uint16_t rgbc[4];

char * IDENTIFIER = (char*) calloc(16, sizeof(byte));
 
void setup() {
  Serial.begin(9600);
  int ident_eeprom_address = 0;
  
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  
  // Reading the ident from EEPROM
  byte *ptr = (byte *) IDENTIFIER;
  for (byte i = 0; i < 16; i++)
    *ptr++ = EEPROM.read(ident_eeprom_address + i);
}

void loop() {
  
  tcs.getRawData(&rgbc[0],&rgbc[1],&rgbc[2],&rgbc[3]);
  
  rgbc[0] ^= B10101010;
  rgbc[1] ^= B10101010;
  rgbc[2] ^= B10101010;
  rgbc[3] ^= B10101010;
  
  if (Serial.available()) {
    char c = Serial.read();
    
    switch (c) {
      case 't':
      case 'T':
        Serial.print("COLOR           ");
        break;
        
      case 'g':
      case 'G':
        Serial.write((byte*) rgbc, 4*sizeof(uint16_t));
        Serial.write((byte*) rgbc, 4*sizeof(uint16_t));
        break;
        
      case 'h':
      case 'H':
        Serial.print("R: ");
        Serial.print(rgbc[0]);
        Serial.print(" G: ");
        Serial.print(rgbc[1]);
        Serial.print(" B: ");
        Serial.print(rgbc[2]);
        Serial.print(" C: ");
        Serial.println(rgbc[3]);
        break;
      
      case 'i':
        Serial.write(IDENTIFIER, 16);
        break;

      case 'I':
        Serial.readBytes(IDENTIFIER, 16);
        // Put the identifier in the first 16 bytes after the calibration data
        int eeprom_address = 0;

        char* ptr = IDENTIFIER;

        for (byte i = 0; i < 16; i++)
          EEPROM.write(eeprom_address + i, *ptr++);

        break;
    }
  }
  
  
}
