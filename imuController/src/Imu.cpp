//
//  Imu.cpp
//  imuMoCap
//
//  Created by Johan Bichel Lindegaard on 10/04/14.
//
//

#include "Imu.h"

Imu::Imu() {
    serialSetup = false;
}

Imu::~Imu() {
    serial.close();
}


bool Imu::toggleLED() {
    if (serial.isInitialized()) {
        serial.writeByte('l');
        serial.writeByte(0x10);
        return true;
    } else {
        return false;
    }
}

void Imu::setup(int _deviceId) {
    deviceId = _deviceId;
    deviceName = serial.getDeviceList().at(deviceId).getDeviceName();
    
    serial.setup(deviceId, 9600);
    serial.flush();
    
    serialSetup = true;
    
    calTime = 10000;
    firstData = 0;
    dataReceived = false;
    
    state = 0;
}


void Imu::update() {
    if (!serialSetup) return;
    
    serial.writeByte('g');
    
    
    // we want to read 16 bytes
    int bytesRequired = 16;
    unsigned char bytes[bytesRequired];
    
    int bytesRemaining = bytesRequired;
    // loop until we've read everything
    while ( bytesRemaining > 0 )
    {
        // check for data
        if ( serial.available() > 0 )
        {
            // try to read - note offset into the bytes[] array, this is so
            // that we don't overwrite the bytes we already have
            int bytesArrayOffset = bytesRequired - bytesRemaining;
            int result = serial.readBytes(&bytes[bytesArrayOffset],
                                          bytesRemaining );
        
            // check for error code
            if (result == OF_SERIAL_ERROR )
            {
                // something bad happened
                ofLog( OF_LOG_ERROR, "unrecoverable error reading from serial" );
                // bail out
                break;
            }
            else if ( result == OF_SERIAL_NO_DATA )
            {
                // nothineg was read, try again
            }
            else
            {
                // we read some data!
                bytesRemaining -= result;
                
            }
        }
    }
    
    bool good = true;
    float floats[4];
    float f;
    
    int br = 0;
    for(int i=0; i<4; i++) {
        unsigned char bl[4];
        for (int b=0; b<4; b++) {
            bl[b] = bytes[br];
            br++;
        }
        memcpy(&floats[i], &bl, sizeof(f));
        
        if(floats[i] > 1 || floats[i] < -1) good = false;
    }
    
    if(good) {
        quaternion.set(floats[0], floats[1], floats[2], floats[3]);
        printf("%.02f %.02f %.02f %.02f \n", floats[0], floats[1], floats[2], floats[3]);
    }
    
}
