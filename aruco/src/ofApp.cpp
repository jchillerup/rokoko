#include "ofApp.h"
#include "ofxCv.h"
#include "ofBitmapFont.h"

void drawMarker(float size, const ofColor & color){
	ofDrawAxis(size);
	ofPushMatrix();
    // move up from the center by size*.5
    // to draw a box centered at that point
    ofTranslate(0,size*0.5,0);
    ofFill();
    ofSetColor(color,50);
    ofDrawBox(size);
    ofNoFill();
    ofSetColor(color);
    ofDrawBox(size);
	ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	useVideo = false;
	string boardName = "boardConfiguration.yml";
    
	if(useVideo){
		player.loadMovie("videoboard.mp4");
		player.play();
		video = &player;
	}else{
		grabber.setDeviceID(1);
		grabber.initGrabber(640,480);
		video = &grabber;
	}
    
	//aruco.setThreaded(false);

	aruco.setup("intrinsics.int", 640, 480, boardName);
	aruco.getBoardImage(board.getPixelsRef());
	board.update();
    
	showMarkers = true;
	showBoard = true;
	showBoardImage = false;
    
	ofEnableAlphaBlending();
    
	ofPixels pixels;
	ofBitmapStringGetTextureRef().readToPixels(pixels);
	ofSaveImage(pixels,"font.bmp");
    
    camFbo.allocate(grabber.getWidth(), grabber.getHeight());
    worldFbo.allocate(grabber.getWidth(), grabber.getHeight());
    
    sender.setup("127.0.0.1", 7000);
    
    
}

//--------------------------------------------------------------
void testApp::update(){
	video->update();
	if(video->isFrameNew()){
		aruco.detectBoard(video->getPixelsRef());
        
        ofxOscMessage m;
        
        ofxCv::Mat rvec = aruco.getBoard().Rvec;
        ofxCv::Mat tvec = aruco.getBoard().Tvec;
        
        //cout<<rvec<<endl;
        //cout<<tvec<<endl;
        
        m.addFloatArg(rvec.at<float>(0));
        m.addFloatArg(rvec.at<float>(1));
        m.addFloatArg(rvec.at<float>(2));
        
        m.addFloatArg(tvec.at<float>(0));
        m.addFloatArg(tvec.at<float>(1));
        m.addFloatArg(tvec.at<float>(2));
        
        //m.addFloatArg(tvec);
        
        sender.sendMessage(m);
        
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(255);
    
    camFbo.begin();
    
    ofSetColor(255);
    
    ofTranslate(camFbo.getWidth()/2, camFbo.getHeight()/2);
    ofRotateX(180);
    ofTranslate(-camFbo.getWidth()/2, -camFbo.getHeight()/2);
	video->draw(0,0);
    ofRotateX(-180);
    
	if(showMarkers){
		for(int i=0;i<aruco.getNumMarkers();i++){
			aruco.begin(i);
			drawMarker(0.15,ofColor::white);
			aruco.end();
		}
	}
    
	if(showBoard && aruco.getBoardProbability()>0.03){
		aruco.beginBoard();
		drawMarker(.5,ofColor::red);
        
        
        ofxCv::Mat rvec = aruco.getBoard().Rvec;
        ofxCv::Mat tvec = aruco.getBoard().Tvec;
        
        ofxCv::Point3f pos = aruco.camParams.getCameraLocation(rvec, tvec);
        
        //cout<<pos<<endl;
        ofSetColor(255);
        ofFill();
        ofDrawBox(pos.x, pos.y, pos.z, 0.05, 0.05, 0.05);
        
		aruco.end();
	}
    
    
    
    
    camFbo.end();
    
    
    worldFbo.begin();
    
    ofBackground(0);
    
    
    cam.begin();
    
    ofNoFill();
    ofDrawBox(0,0,0, 300, 300, 300);
    
    ofPushMatrix();{
        
        ofxCv::Mat rvec = aruco.getBoard().Rvec;
        ofxCv::Mat tvec = aruco.getBoard().Tvec;
        
        ofxCv::Point3f pos = aruco.camParams.getCameraLocation(rvec, tvec);
        
        //cout<<pos<<endl;
        ofRotateY(90);
        ofDrawBox(pos.x*100, pos.y*100, pos.z*100, 20, 20, 20);
        
        
    }ofPopMatrix();
    
    cam.end();
    
    worldFbo.end();
    
    ofSetColor(255);
    camFbo.draw(0,0);
    worldFbo.draw(camFbo.getWidth(),0);
    
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(0, camFbo.getHeight()+20);
	ofDrawBitmapString("markers detected: " + ofToString(aruco.getNumMarkers()),20,20);
	ofDrawBitmapString("fps " + ofToString(ofGetFrameRate()),20,40);
	ofDrawBitmapString("m toggles markers",20,60);
	ofDrawBitmapString("b toggles board",20,80);
	ofDrawBitmapString("i toggles board image",20,100);
	ofDrawBitmapString("s saves board image",20,120);
	ofDrawBitmapString("0-9 saves marker image",20,140);
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(key=='m') showMarkers = !showMarkers;
	if(key=='b') showBoard = !showBoard;
	if(key=='i') showBoardImage = !showBoardImage;
	if(key=='s') board.saveImage("boardimage.png");
	if(key>='0' && key<='9'){
	    // there's 1024 different markers
	    int markerID = key - '0';
	    aruco.getMarkerImage(markerID,240,marker);
	    marker.saveImage("marker"+ofToString(markerID)+".png");
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
    
}
