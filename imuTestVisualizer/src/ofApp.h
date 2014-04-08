#pragma once

#include "ofMain.h"
#include "ofxSkeleton.h"
#include "ofxUi.h"


class ofApp : public ofBaseApp{
	
	typedef shared_ptr<pal::ofxSkeleton::ofxJoint>  JointP_t;
	
	//vector<JointP_t>							mSkeleton;
	
	ofEasyCam										mCam1;
	map<string, JointP_t>							mSkeleton;
    

public:
	void setup();
	void update();
	void draw();
	
    void exit();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
    
    ofSerial serial;
    
    // data from IMU
    ofQuaternion rawQuarternion;
    ofVec3f rawGyro;
    ofVec3f rawAccel;
    ofVec3f rawMag;
    
    ofQuaternion dmpQuaternion;
    ofVec3f dmpEulerPose;
    ofVec3f calAccel;
    ofVec3f calMag;
    
    ofQuaternion fusedQuaternion;
    ofVec3f fusedEulerPose;
    
    ofVec3f residualAccel;
    ofVec3f velocity;
    ofVec3f position;
    
    ofVec3f calZeroVector;
    
    ofEasyCam cam;
    
    ofxUISuperCanvas *gui;
    float *buffer;
    ofxUIMovingGraph *mg;
    void guiEvent(ofxUIEventArgs &e);
    
    int long calTime = 10000;
    int long firstData = 0;
    bool dataReceived = false;
    
    ofVec3f offset = ofVec3f(0,0,0);
    
    int state = 0; // 0 waiting for data - 1 calibrating - 2 receiving - 3 error
	
};

