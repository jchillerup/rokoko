/*
*  camera calibration
*  stefanix - helloworld@stefnix.net
*/

#include "testApp.h"
 


void testApp::setup() {
    ofSetVerticalSync( true );
    ofSetWindowPosition( 0,0 );
    cwidth = 320;
    cheight = 240;
    csize = cvSize( cwidth,cheight );
	vidGrabber.initGrabber( cwidth,cheight );
    colorImg.allocate( cwidth,cheight );
    undistortedImg.allocate( cwidth,cheight );

    calib.allocate( csize, 7, 7 );
    shotToShow = -1;
    bUndistortLiveFeed = false;

}



void testApp::update() {
	ofBackground( 100,100,100 );
    vidGrabber.grabFrame();
    if( vidGrabber.isFrameNew() ) {
        colorImg.setFromPixels(vidGrabber.getPixels(), cwidth,cheight );
        if( bUndistortLiveFeed ) {
            undistortedImg = colorImg;
            undistortedImg.undistort( calib.distortionCoeffs[0], calib.distortionCoeffs[1],
                                      calib.distortionCoeffs[2], calib.distortionCoeffs[3],
                                      calib.camIntrinsics[0], calib.camIntrinsics[4],
                                      calib.camIntrinsics[2], calib.camIntrinsics[5] );      
        }
    }
}



void testApp::draw() {
    ofSetColor(0xffffff);
    colorImg.draw( 20,20, 320,240 );
    if( bUndistortLiveFeed ) {
        undistortedImg.draw( 360,20, 320,240 );
    }
    
    
    if( calib.colorImages.size() > shotToShow ) {
        drawImage( calib.colorImages[shotToShow], 20,280, 320,240 );
    }

    if( calib.undistortedImg.size() > shotToShow ) {    
        drawImage( calib.undistortedImg[shotToShow], 360,280, 320,240 );
    }
    
    // REPORT
    ofSetColor( 0xffffff );    
    ostringstream docstring;
    docstring << "[space] to add an image - only recognizable patters are added" << endl
              << "[c] to calibrate with added images" << endl
              << "[<], [>] to view added images" << endl
              << "Framerate: " << ofGetFrameRate() << endl
              << "Showing Image# " << shotToShow << endl;
    ofDrawBitmapString( docstring.str(), 20,ofGetHeight()-70 );  
}



void testApp::drawImage(IplImage* image, int x, int y, int width, int height) {
    IplImage* o;
    if( image->nChannels == 3 )
        o = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 3 );
    else if( image->nChannels == 1 )
        o = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 1 );
    else {
        printf( "unsupported format into imageToBackground()" );
        //exit(0);
    }
    cvResize( image, o, CV_INTER_NN );
    cvFlip( o, o, 0 );
    glRasterPos3f( x, y+height, 0.0 );
    if( o->nChannels == 3 )
        glDrawPixels( o->width, o->height , 
                     GL_BGR, GL_UNSIGNED_BYTE, o->imageData );
    else if( o->nChannels == 1 )
        glDrawPixels( o->width, o->height , 
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, o->imageData );
    cvReleaseImage( &o );
}




void testApp::keyPressed( int key ) {
    if( key == 's' ) {
        vidGrabber.videoSettings();
    } else if( key == ' ' ) {
        if( calib.addImage( colorImg.getCvImage() ) ) {
            shotToShow++;
        }
    } else if( key == 'c' ) {
        cout << "Calibrating..." << endl;
        calib.calibrate();
        calib.undistort();

        bUndistortLiveFeed = true;
        
    } else if( key == ',' ) {
        shotToShow = MAX(shotToShow-1, 0);    
    } else if( key == '.' ) {
        shotToShow = MIN(shotToShow+1, calib.colorImages.size()-1);    
    }    
}
void testApp::mouseMoved( int x, int y ) {}
void testApp::mouseDragged( int x, int y, int button ) {}
void testApp::mousePressed( int x, int y, int button ) {}
void testApp::mouseReleased() {}


