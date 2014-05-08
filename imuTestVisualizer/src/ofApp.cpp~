
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
    
    
    string namesUpper[4] = {"Shoulder", "Elbow", "Hand", "Finger"};
    for (int i = 0; i < 4; i++){
        mSkeleton["L_" + namesUpper[i]] = JointP_t(new ofxJoint());
        mSkeleton["R_" + namesUpper[i]] = JointP_t(new ofxJoint());
    }
    
    // apply a limb's hierarchy
    mSkeleton["L_Finger"]->bone(mSkeleton["L_Hand"])->bone(mSkeleton["L_Elbow"])->bone(mSkeleton["L_Shoulder"])->bone(mSkeleton["Chest"]);
    
    mSkeleton["R_Finger"]->bone(mSkeleton["R_Hand"])->bone(mSkeleton["R_Elbow"])->bone(mSkeleton["R_Shoulder"])->bone(mSkeleton["Chest"]);
    
    // set the limb's joints positions
    mSkeleton["L_Shoulder"]->setGlobalPosition(ofVec3f(-60, 100, 0 ));
    mSkeleton["L_Shoulder"]->setOrientation(ofQuaternion(20 - 40, ofVec3f(0,1,0)));
    mSkeleton["L_Elbow"]->setPosition(ofVec3f(-20, -80, 0));
    mSkeleton["L_Hand"]->setPosition(ofVec3f(0, -80, 0));
    mSkeleton["L_Finger"]->setPosition(ofVec3f(0, -20, 0));
    
    mSkeleton["R_Shoulder"]->setGlobalPosition(ofVec3f(60, 100, 0 ));
    mSkeleton["R_Shoulder"]->setOrientation(ofQuaternion(20 - 40, ofVec3f(0,-1,0)));
    mSkeleton["R_Elbow"]->setPosition(ofVec3f(20, -80, 0));
    mSkeleton["R_Hand"]->setPosition(ofVec3f(0, -80, 0));
    mSkeleton["R_Finger"]->setPosition(ofVec3f(0, -20, 0));
    
    
	// set joint names according to their map indices.
    for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
		it->second->setName(it->first);
	}
    
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
    
    
    //elbow->setOrientation(fusedQuaternion);
    
    //mg->addPoint(buffer[0]);
    //for(int i = 0; i < 256; i++) { buffer[i] = ofNoise(i/100.0, ofGetElapsedTimef()); }
    
    mSkeleton["L_Elbow"]->setOrientation(fusedQuaternion);
    
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
    //  ofScale(3,3,3);
    
    ofSetColor(255,255,255);
    
    //hand->draw();
    //elbow->draw();
    //shoulder->draw();
    
    for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
		it->second->draw(10);
	}
    
    ofPopMatrix();
	
	cam.end();
    
    if (shouldDrawLabels) {
		for (map<string, JointP_t>::iterator it = mSkeleton.begin(); it != mSkeleton.end(); ++it){
			ofDrawBitmapString(it->second->getName(), cam.worldToScreen(it->second->getGlobalPosition()) * ofVec3f(1,1,0) + ofVec3f(10,10));
		}
	}
    
    
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


