/*
*  camera calibration
*  stefanix - helloworld@stefnix.net
*/

#include <iostream>
#include <vector>


#include "ofMain.h"
#include "ofCvMain.h"
#include "ofCvCameraCalibration.h"
#include "ofVectorMath.h"



class testApp : public ofSimpleApp {
  public:
	void setup();
	void update();
	void draw();
	void keyPressed( int key );
	void mouseMoved( int x, int y );
	void mouseDragged( int x, int y, int button );
	void mousePressed( int x, int y, int button );
	void mouseReleased();
    void drawImage( IplImage* image, int x, int y, int width, int height );
    
    int cwidth;
    int cheight;
    CvSize csize;
	ofVideoGrabber  vidGrabber;
    ofCvColorImage  colorImg;
    ofCvGrayscaleImage  undistortedImg;

    ofCvCameraCalibration calib;
    int shotToShow;
    bool bUndistortLiveFeed;

};

