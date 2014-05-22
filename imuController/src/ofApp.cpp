#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  
  calTime = 10000;
  firstData = 0;
  dataReceived = false;
  debugdraw = true;

  offset = ofVec3f(0,0,0);

  state = 0; // 0 waiting for data - 1 calibrating - 2 receiving - 3 error

  serial.listDevices();
  vector<ofSerialDeviceInfo> devices = serial.getDeviceList();
  
  int serialAdresses [] = {0
                            ,1
                            ,2
                            ,3
                            ,4
  };

  for(int i=0; i<NUM_SENSORS; i++) {
    Imu * imu = new Imu();

    imu->setup(serialAdresses[i]);
    imus.push_back(imu);
      
  }

  oscSender.setup("192.168.1.50", 14040);
  //serial.setup(0, 57600);
  //serial.flush();
}

//--------------------------------------------------------------
void ofApp::update(){

  for(int i=0; i<imus.size(); i++) {
    imus[i]->update();
  }

  for(int i=0; i<imus.size(); i++) {

    ofxOscMessage m;
    //m.setAddress("imu");
    m.setAddress("sensor");
    m.addStringArg(imus[i]->deviceName);
    //m.addIntArg(imus[i]->deviceId); //Todo: should not be device ID but a constant id locked to a body part
    m.addFloatArg(imus[i]->quaternion.x());
    m.addFloatArg(imus[i]->quaternion.y());
    m.addFloatArg(imus[i]->quaternion.z());
    m.addFloatArg(imus[i]->quaternion.w());
    //printf("osc send");
    
    oscSender.sendMessage(m);
  }

}

//--------------------------------------------------------------
void ofApp::draw(){
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){


}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

void ofApp::exit(){


}
