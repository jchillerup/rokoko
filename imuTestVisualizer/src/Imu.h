//
//  Imu.h
//  imuMoCap
//
//  Created by Johan Bichel Lindegaard on 10/04/14.
//
//

#pragma once

#include "ofMain.h"

class Imu {
public:
    // address
    // when to pull slave select pin high
    
    Imu();
    ~Imu();
    
    ofQuaternion quaternion;    
    ofSerial serial;
    void setup(int _deviceId);
    void update();

    int deviceId;

    int long calTime;
    int long firstData;
    bool dataReceived;
    
    int state;
    /*
     ofQuaternion rawQuarternion;
     ofVec3f rawGyro;
     ofVec3f rawAccel;
     ofVec3f rawMag;
     ofQuaternion dmpQuaternion;
     ofVec3f dmpEulerPose;
     ofVec3f calAccel;
     ofVec3f calMag;
     ofVec3f fusedEulerPose;
     ofVec3f residualAccel;
     */
    
private:
    void updateQuaternion(string str, ofQuaternion * quat, string delimiter);
};
