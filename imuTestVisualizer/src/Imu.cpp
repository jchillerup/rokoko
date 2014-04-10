//
//  Imu.cpp
//  imuMoCap
//
//  Created by Johan Bichel Lindegaard on 10/04/14.
//
//

#include "Imu.h"

string ofxTrimStringRight(string str) {
    size_t endpos = str.find_last_not_of(" \t\r\n");
    return (string::npos != endpos) ? str.substr( 0, endpos+1) : str;
}

// trim trailing spaces
string ofxTrimStringLeft(string str) {
    size_t startpos = str.find_first_not_of(" \t\r\n");
    return (string::npos != startpos) ? str.substr(startpos) : str;
}

string ofxTrimString(string str) {
    return ofxTrimStringLeft(ofxTrimStringRight(str));;
}

string ofxGetSerialString(ofSerial &serial, char until) {
    static string str;
    stringstream ss;
    char ch;
    int ttl=1000;
    while ((ch=serial.readByte())>0 && ttl-->0 && ch!=until) {
        ss << ch;
    }
    str+=ss.str();
    if (ch==until) {
        string tmp=str;
        str="";
        return ofxTrimString(tmp);
    } else {
        return "";
    }
}

void updateQuaternion(string str, ofQuaternion * quat, string delimiter = "!") {
    if(str.length() > 3) {
        vector<string> floats = ofSplitString(str, delimiter);
        quat->set(ofToFloat(floats[0]), ofToFloat(floats[2]), -ofToFloat(floats[3]), ofToFloat(floats[1]));
    }
    //quat->set(ofToFloat(floats[0]), ofToFloat(floats[1]), ofToFloat(floats[2]), ofToFloat(floats[3]));
}


void updateVector(string str, ofVec3f * vec, string delimiter = "!") {
    if(str.length() > 2) {
        vector<string> floats = ofSplitString(str, delimiter);
        vec->x = ofToFloat(floats[0]);
        vec->y = ofToFloat(floats[2]);
        vec->z = ofToFloat(floats[1]);
    }
    
}


Imu::Imu() {
}

Imu::~Imu() {
    serial.close();
}


void Imu::setup(int _deviceId) {
    deviceId = _deviceId;
    
    serial.setup(deviceId, 57600);
    serial.flush();
}


void Imu::update() {
    
    if(serial.available() > 0) {
        
        vector<string> indata;
        indata.clear();
        string in = ofxGetSerialString(serial, '&');
        indata = ofSplitString(in, "\n");
        
        
        if(indata.size() == 3) {
            
            if(indata[0] == "DS") {
                updateQuaternion(indata[1], &quaternion);
                
                if(!dataReceived) {
                    state = 1;
                    //This is the first time we get data
                    firstData = ofGetElapsedTimeMillis();
                    dataReceived = true;
                    //offset = residualAccel;
                }
                
            }
            indata.clear();
        }
        serial.flush();
    }
    
    
    if(dataReceived && (ofGetElapsedTimeMillis() - firstData) < calTime) {
        
        // calibration stage for 3 seconds
        //calZeroVector = residualAccel;
        //offset = (offset * 59.0/60.0) + (residualAccel + 1.0/60.0);
        
    } else if (dataReceived) {
        state = 2;
        // running stage
        
        //residualAccel += (residualAccel - offset) * 0.0001;
        //velocity += residualAccel * 0.001;
        //position += velocity * 0.01;
        
        
    }
    
}