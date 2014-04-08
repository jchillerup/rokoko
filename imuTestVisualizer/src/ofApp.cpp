
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
    serial.setup(0, 57600);
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
    
    gui = new ofxUISuperCanvas("Rokoko MoCap");
    
    vector<float> buffer;
    for(int i = 0; i < 256; i++)
    {
        buffer.push_back(0.0);
    }
    
    //gui->addLabel("MOVING GRAPH", OFX_UI_FONT_MEDIUM);
    //mg = gui->addMovingGraph("MOVING", buffer, 256, 0.0, 1.0);

    gui->setPosition(0, 0);
    gui->autoSizeToFitWidgets();
    
	ofAddListener(gui->newGUIEvent,this,&ofApp::guiEvent);
    
    position = ofVec3f(0,0,0);
    velocity = ofVec3f(0,0,0);
    
    
}



//--------------------------------------------------------------
void ofApp::update(){
	
    
    if(serial.available() > 0) {
        
        vector<string> indata;
        indata.clear();
        string in = ofxGetSerialString(serial, '&');
        indata = ofSplitString(in, "\n");
        
        
        if(indata.size() == 3) {
            
            if(indata[0] == "DS") {
                updateQuaternion(indata[1], &fusedQuaternion);
                updateVector(indata[2], &residualAccel);
                
                if(!dataReceived) {
                    state = 1;
                    //This is the first time we get data
                    firstData = ofGetElapsedTimeMillis();
                    dataReceived = true;
                    offset = residualAccel;
                }
        
            }
            indata.clear();
        }
        serial.flush();
    }
    
    
    if(dataReceived && (ofGetElapsedTimeMillis() - firstData) < calTime) {
        
        // calibration stage for 3 seconds
        //calZeroVector = residualAccel;
        offset = (offset * 59.0/60.0) + (residualAccel + 1.0/60.0);
        
        
    } else if (dataReceived) {
        state = 2;
        // running stage
        
        residualAccel += (residualAccel - offset) * 0.0001;
        velocity += residualAccel * 0.001;
        position += velocity * 0.01;
        
        
    }
    
    
    elbow->setOrientation(fusedQuaternion);
    
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
    
    ofTranslate(400, 0);
    
    ofVec3f qaxis; float qangle;
    fusedQuaternion.getRotate(qangle, qaxis);
    ofRotate(qangle, qaxis.x, qaxis.y, qaxis.z);
    
    //ofDrawBox(0, 0, 0, 100, 100, 100);
    ofSetColor(0,0,200);
    ofDrawCone(0, 0, 0, 10, 90);
    
	ofPopMatrix();
    
    ofPushMatrix();
    ofScale(3,3,3);
    
    ofSetColor(255,255,255);
    hand->draw();
    elbow->draw();
    shoulder->draw();
    
    ofPopMatrix();
	
	cam.end();
    
    if(state==1){
        ofSetColor(255, 255, 255);
        ofDrawBitmapString("Clibrating", 100,100);
    } else if(state == 2) {
        ofSetColor(0, 255, 0);
        ofDrawBitmapString("Receiving data", 100,100);
    } else if(state==0) {
        ofSetColor(255, 0, 0);
        ofDrawBitmapString("Waiting for data", 100,100);
    }

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


