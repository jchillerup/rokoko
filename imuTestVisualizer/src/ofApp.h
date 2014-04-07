#pragma once

#include "ofMain.h"
#include "ofxSkeleton.h"
#include "ofxUi.h"


class ofApp : public ofBaseApp{
	
	typedef shared_ptr<pal::ofxSkeleton::ofxJoint>  JointP_t;
	
	vector<JointP_t>							mSkeleton;
        
	JointP_t hand;
    JointP_t elbow;
    JointP_t shoulder;
	
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
    
    ofEasyCam cam;
    
    ofxUISuperCanvas *gui;
    float *buffer;
    ofxUIMovingGraph *mg;
    void guiEvent(ofxUIEventArgs &e);
	
};

