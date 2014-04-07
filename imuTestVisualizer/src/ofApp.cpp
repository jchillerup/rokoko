
#include "ofApp.h"

using namespace pal::ofxSkeleton;

u_long animCurrentFrame;
bool animIsPaused = false;
bool shouldDrawLabels = false;
bool shouldDrawInfo = true;


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
};


void updateVector(string str, ofVec3f * vec, string delimiter = "!") {
    if(str.length() > 2) {
        vector<string> floats = ofSplitString(str, delimiter);
        vec->x = ofToFloat(floats[0]);
        vec->y = ofToFloat(floats[2]);
        vec->z = ofToFloat(floats[1]);
    }
    
};


//--------------------------------------------------------------
void ofApp::setup(){
	
	
    serial.listDevices();
    serial.setup(0, 115200);
    serial.flush();
    
	animIsPaused = false;
	animCurrentFrame = 0;
	
	ofDisableSmoothing();
	ofDisableAlphaBlending();
	
	//cam.setupPerspective(false, 60, 0.1, 3000);
//	mCam1.setPosition(500, 800, 800);
	cam.lookAt(ofVec3f(0));

	
    for (int i = 0; i < 3; i++){
        mSkeleton.push_back(JointP_t(new ofxJoint));
    }
    
    hand = mSkeleton[0];
    hand->setName("Hand");
    
    elbow = mSkeleton[1];
    elbow->setName("Elbow");
    
    shoulder = mSkeleton[2];
    shoulder->setName("Shoulder");
	
    hand->bone(elbow)->bone(shoulder);
    
    shoulder->setGlobalPosition(ofVec3f(0,0,0));
    elbow->setGlobalPosition(ofVec3f(50,0,0));
    hand->setGlobalPosition(ofVec3f(0,-50,0));
	// set joint names according to their map indices.
    
    gui = new ofxUISuperCanvas("PANEL");
    
    vector<float> buffer;
    for(int i = 0; i < 256; i++)
    {
        buffer.push_back(0.0);
    }
    
    gui->addLabel("MOVING GRAPH", OFX_UI_FONT_MEDIUM);
    mg = gui->addMovingGraph("MOVING", buffer, 256, 0.0, 1.0);

    gui->setPosition(0, 0);
    gui->autoSizeToFitWidgets();
    
	ofAddListener(gui->newGUIEvent,this,&ofApp::guiEvent);
    
}



//--------------------------------------------------------------
void ofApp::update(){
	
    
    if(serial.available() > 0) {
        
        vector<string> indata;
        indata.clear();
        string in = ofxGetSerialString(serial, '&');
        indata = ofSplitString(in, "\n");
        
        
        if(indata.size() == 11) {
            //cout<<indata[0]<<endl;
            
            /*for(int i=0; i<indata.size();i++) {
                ofStringReplace(indata[i], "\t", "");
            }*/
            
            if(indata[0] == "DATASTART") {
                /*for(int i=0; i<indata.size();i++) {
                    cout<<indata[i]<<endl;
                }*/
                
                updateQuaternion(indata[1], &rawQuarternion);
                updateVector(indata[2], &rawGyro);
                updateVector(indata[3], &rawAccel);
                updateVector(indata[4], &rawMag);
            
                updateQuaternion(indata[5], &dmpQuaternion);
            
                updateVector(indata[6], &dmpEulerPose);
                updateVector(indata[7], &calAccel);
                updateVector(indata[8], &calMag);
            
                updateVector(indata[9], &fusedEulerPose);
                updateQuaternion(indata[10], &fusedQuaternion);
        
            }
            indata.clear();
        }
        serial.flush();
    }
    
    
    //mg->addPoint(buffer[0]);
    //for(int i = 0; i < 256; i++) { buffer[i] = ofNoise(i/100.0, ofGetElapsedTimef()); }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	ofEnableDepthTest();
	
	ofBackground(0);
    
    ofNoFill();
    ofSetColor(255);
	cam.begin();
    //ofDrawGrid(600);
    
	ofPushMatrix();
    /*
    hand->draw();
    elbow->draw();
    shoulder->draw();
     */
    
    ofVec3f qaxis; float qangle;
    fusedQuaternion.getRotate(qangle, qaxis);
    ofRotate(qangle, qaxis.x, qaxis.y, qaxis.z);
    
    //ofDrawBox(0, 0, 0, 100, 100, 100);
    ofDrawCone(0, 0, 0, 20, 180);
    
	ofPopMatrix();
    
    
    ofPushMatrix();
    ofTranslate(200, 0);
    dmpQuaternion.getRotate(qangle, qaxis);
    ofRotate(qangle, qaxis.x, qaxis.y, qaxis.z);
    
    //ofDrawBox(0, 0, 0, 100, 100, 100);
    ofDrawCone(0, 0, 0, 20, 180);
    
	ofPopMatrix();
    
    ofPushMatrix();
    
    ofTranslate(-200, 0);
    ofRotateX(ofRadToDeg(fusedEulerPose.x));
    ofRotateY(ofRadToDeg(fusedEulerPose.y));
    ofRotateZ(ofRadToDeg(fusedEulerPose.z));
    
    ofSetColor(255,0,0);
    ofDrawCone(0, 0, 0, 20, 180);
    
	ofPopMatrix();
	
	cam.end();
    

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	
	
	switch (key) {
		case ' ':
			animIsPaused ^= true;
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
	serial.close();
}

void ofApp::guiEvent(ofxUIEventArgs &e)
{
}


