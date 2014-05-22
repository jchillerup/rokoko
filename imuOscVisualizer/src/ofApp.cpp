#include "ofApp.h"

using namespace pal::ofxSkeleton;

u_long animCurrentFrame;
bool animIsPaused = false;
bool shouldDrawLabels = false;
bool shouldDrawInfo = true;

//--------------------------------------------------------------
void ofApp::setup(){
  gui = new ofxUICanvas();

  oscReceiver.setup(14040);

  animIsPaused = false;
  animCurrentFrame = 0;

  ofDisableSmoothing();
  ofDisableAlphaBlending();

  //cam.setupPerspective(false, 60, 0.1, 3000);
  //mCam1.setPosition(500, 800, 800);
  cam.lookAt(ofVec3f(0));

  // Pelvis is root Maybe we should have another point on torso?
    
  mSkeleton["Pelvis"] = JointP_t(new ofxJoint());
  mSkeleton["Pelvis"]->setGlobalPosition(ofVec3f(0));
    
  mSkeleton["Chest"] = JointP_t(new ofxJoint());
  mSkeleton["Chest"]->setParent(mSkeleton["Pelvis"]);
  mSkeleton["Chest"]->setPosition(ofVec3f(0, 100 , 0 ));

  mSkeleton["Head"] = JointP_t(new ofxJoint());
  mSkeleton["Head"]->setParent(mSkeleton["Chest"]);
  mSkeleton["Head"]->setPosition(ofVec3f(0, 60 , 0 ));

  string names[4] = {"Hip", "Knee", "Foot", "Toe"};

  for (int i = 0; i < 4; i++){
    mSkeleton["L_" + names[i]] = JointP_t(new ofxJoint());
    mSkeleton["R_" + names[i]] = JointP_t(new ofxJoint());
  }

  // apply a limb's hierarchy
  mSkeleton["L_Toe"]->bone(mSkeleton["L_Foot"])->bone(mSkeleton["L_Knee"])->bone(mSkeleton["L_Hip"])->bone(mSkeleton["Pelvis"]);

  mSkeleton["R_Toe"]->bone(mSkeleton["R_Foot"])->bone(mSkeleton["R_Knee"])->bone(mSkeleton["R_Hip"])->bone(mSkeleton["Pelvis"]);

  // set the limb's joints positions
  mSkeleton["L_Hip"]->setGlobalPosition(ofVec3f(-40, -80 , 0 ));
  mSkeleton["L_Hip"]->setOrientation(ofQuaternion(20 - 40, ofVec3f(0,1,0)));
  mSkeleton["L_Knee"]->setPosition(ofVec3f(0, -100, 0));
  mSkeleton["L_Foot"]->setPosition(ofVec3f(0, -80, 0));
  mSkeleton["L_Toe"]->setPosition(ofVec3f(0, -5, 20));

  mSkeleton["R_Hip"]->setGlobalPosition(ofVec3f(40, -80 , 0 ));
  mSkeleton["R_Hip"]->setOrientation(ofQuaternion(20 - 40, ofVec3f(0,-1,0)));
  mSkeleton["R_Knee"]->setPosition(ofVec3f(0, -100, 0));
  mSkeleton["R_Foot"]->setPosition(ofVec3f(0, -80, 0));
  mSkeleton["R_Toe"]->setPosition(ofVec3f(0, -5, 20));


 /* string namesUpper[4] = {"Shoulder", "Elbow", "Hand", "Finger"};
  for (int i = 0; i < 4; i++){
    mSkeleton["L_" + namesUpper[i]] = JointP_t(new ofxJoint());
    mSkeleton["R_" + namesUpper[i]] = JointP_t(new ofxJoint());
  }*/
    
    mSkeleton["01"] = JointP_t(new ofxJoint());
    mSkeleton["02"] = JointP_t(new ofxJoint());
    mSkeleton["03"] = JointP_t(new ofxJoint());
    mSkeleton["04"] = JointP_t(new ofxJoint());
    mSkeleton["05"] = JointP_t(new ofxJoint());
    mSkeleton["06"] = JointP_t(new ofxJoint());
    mSkeleton["07"] = JointP_t(new ofxJoint());
    mSkeleton["08"] = JointP_t(new ofxJoint());

  // apply a limb's hierarchy
  mSkeleton["04"]->bone(mSkeleton["03"])->bone(mSkeleton["02"])->bone(mSkeleton["01"])->bone(mSkeleton["Chest"]);

  mSkeleton["08"]->bone(mSkeleton["07"])->bone(mSkeleton["06"])->bone(mSkeleton["05"])->bone(mSkeleton["Chest"]);

  // set the limb's joints positions
  mSkeleton["01"]->setGlobalPosition(ofVec3f(-60, 100, 0 ));
  mSkeleton["01"]->setOrientation(ofQuaternion(20 - 40, ofVec3f(0,1,0)));
  mSkeleton["02"]->setPosition(ofVec3f(-20, -80, 0));
  mSkeleton["03"]->setPosition(ofVec3f(0, -80, 0));
  mSkeleton["04"]->setPosition(ofVec3f(0, -20, 0));

  mSkeleton["05"]->setGlobalPosition(ofVec3f(60, 100, 0 ));
  mSkeleton["05"]->setOrientation(ofQuaternion(20 - 40, ofVec3f(0,-1,0)));
  mSkeleton["06"]->setPosition(ofVec3f(20, -80, 0));
  mSkeleton["07"]->setPosition(ofVec3f(0, -80, 0));
  mSkeleton["08"]->setPosition(ofVec3f(0, -20, 0));

  // set joint names according to their map indices.
  for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
    it->second->setName(it->first);
    filters[it->first] = BQF(new ofxBiquadFilter4f(OFX_BIQUAD_TYPE_LOWPASS, 0.5, 0.7, 0.0));
  }

  position = ofVec3f(0,0,0);
  velocity = ofVec3f(0,0,0);
}

//--------------------------------------------------------------
void ofApp::update(){

    while(oscReceiver.hasWaitingMessages()) {
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        
        //cout<<" "<<m.getAddress()<<" "<<m.getArgAsString(0)<<endl;
        
        if(m.getAddress() == "/sensor") {
            
            ofVec4f q = ofVec4f(m.getArgAsFloat(4), m.getArgAsFloat(1), m.getArgAsFloat(2), m.getArgAsFloat(3));
            string id = m.getArgAsString(0).substr(0,2);
            
            mSkeleton[id]->setOrientation(filters[id]->update(q));
            
           /* if(string("01").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["01"]->setOrientation(q);
                
            } else if (string("02").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["02"]->setOrientation(q);
            } else if (string("03").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["03"]->setOrientation(q);
            } else if (string("04").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["04"]->setOrientation(q);
            } else if (string("05").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["05"]->setOrientation(q);
            } else if (string("06").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["06"]->setOrientation(q);
            } else if (string("07").compare(0,2,m.getArgAsString(0))) {
                mSkeleton["07"]->setOrientation(q);
            }*/
            
            //cout<<q<<endl;
            
        }
        
        
    }
  
    /*for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
        it->second->setOrientation(filters[it->first])
        
    }*/
    
  //if(debugdraw) {
    //mSkeleton["05"]->setOrientation(imus[0]->quaternion);
    // mSkeleton["06"]->setOrientation(imus[1]->quaternion);
    // mSkeleton["07"]->setOrientation(imus[2]->quaternion);
    
  //}

}

//--------------------------------------------------------------
void ofApp::draw(){

  ofEnableDepthTest();


  if(debugdraw) {

    ofBackground(0);
    ofNoFill();
    ofSetColor(255);
    cam.begin();

    // ofDrawGrid(600);
    ofPushMatrix(); {

      ofTranslate(400, 0);

      ofVec3f qaxis; float qangle;
      fusedQuaternion.getRotate(qangle, qaxis);
      ofRotate(qangle, qaxis.x, qaxis.y, qaxis.z);

      ofSetColor(0,0,200);
      ofDrawCone(0, 0, 0, 10, 90);
      
    }ofPopMatrix();

    ofPushMatrix();{

      ofSetColor(255,255,255);
      for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
        it->second->draw(10);
      }

    }ofPopMatrix();

    cam.end();

    if (shouldDrawLabels) {
      for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
        ofDrawBitmapString(it->second->getName(), cam.worldToScreen(it->second->getGlobalPosition()) * ofVec3f(1,1,0) + ofVec3f(10,10));
      }
    }
  }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
  switch (key) {
  case 'd':
    debugdraw = !debugdraw;
    break;
  case 'i':
    shouldDrawInfo ^= true;
    break;
  case 'l':
    shouldDrawLabels ^= true;
    break;
  case 'f':
    ofToggleFullscreen();
    break;
  default:
    break;
  }

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
