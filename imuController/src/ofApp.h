#pragma once

#include "ofMain.h"
#include "Imu.h"
#include "ofxOsc.h"

#define NUM_SENSORS 3

class ofApp : public ofBaseApp{
    

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
    
  int long calTime;
  int long firstData;
  bool dataReceived;
    
  ofVec3f offset;
    
  int state;
    
  vector<Imu *> imus;
    
  bool debugdraw;
    
  ofxOscSender oscSender;
};

