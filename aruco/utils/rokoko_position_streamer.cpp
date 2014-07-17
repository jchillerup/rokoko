/*****************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************/

/**
 * Modifications to fit the ROKOKO project.
 * By Jens Christian Hillerup <jc@bitblueprint.com>
 * 
 * This program, based on aruco_test_board.cpp, streams the R and T vectors of the camera's
 * extrinsics by OSC to a Unity-powered 3D rendering. It is used to determine the position
 * of the camera on the knee of an actor, in order to determine their position on a stage
 * made of ArUco markers.
**/

//define SHOW_THRESHOLDED
//define SHOW_GUI

#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "aruco.h"
#include "lo/lo.h"

using namespace cv;
using namespace aruco;

string TheInputVideo;
string TheIntrinsicFile;
string TheBoardConfigFile;
bool The3DInfoAvailable=false;
float TheMarkerSize=-1;
VideoCapture TheVideoCapturer;
Mat TheInputImage,TheInputImageCopy;
CameraParameters TheCameraParameters;
BoardConfiguration TheBoardConfig;
BoardDetector TheBoardDetector;

string TheOutVideoFilePath;
cv::VideoWriter VWriter;

void cvTackBarEvents(int pos,void*);
pair<double,double> AvrgTime(0,0); //determines the average time required for detection
double ThresParam1,ThresParam2;
int iThresParam1,iThresParam2;
int waitTime=0;

bool readArguments ( int argc,char **argv )
{

  if (argc<3) {
    cerr<<"Invalid number of arguments"<<endl;
    cerr<<"Usage: (in.avi|live) boardConfig.yml [intrinsics.yml] [size]"<<endl;
    return false;
  }
  TheInputVideo=argv[1];
  TheBoardConfigFile=argv[2];
  if (argc>=4)
    TheIntrinsicFile=argv[3];
  if (argc>=5)
    TheMarkerSize=atof(argv[4]);

  if (argc==4)
    cerr<<"NOTE: You need makersize to see 3d info!!!!"<<endl;

  return true;
}

#ifdef SHOW_GUI
void processKey(char k) {
  switch (k) {
  case 's':
    if (waitTime==0) waitTime=10;
    else waitTime=0;
    break;
  }
}
#endif

/************************************
 *
 *
 *
 *
 ************************************/
int main(int argc,char **argv)
{
  try
    {
      if (  readArguments (argc,argv)==false) return 0;
      //parse arguments
      TheBoardConfig.readFromFile(TheBoardConfigFile);

      //read from camera or from  file
      if (TheInputVideo.find("live")!=string::npos) {
        int vIdx=0;
        
        //check if the :idx is here
        char cad[100];
        if (TheInputVideo.find(":")!=string::npos) {
          std::replace(TheInputVideo.begin(),TheInputVideo.end(),':',' ');
          sscanf(TheInputVideo.c_str(),"%s %d",cad,&vIdx);
        }

        cout<<"Opening camera index "<<vIdx<<endl;
        TheVideoCapturer.open(vIdx);
        waitTime=10;
      }
      else {
        TheVideoCapturer.open(TheInputVideo);
      }
      
      //check video is open
      if (!TheVideoCapturer.isOpened()) {
        cerr<<"Could not open video"<<endl;
        return -1;
      }

      //read first image to get the dimensions
      TheVideoCapturer >> TheInputImage;
      
      //read camera parameters if passed
      if (TheIntrinsicFile != "") {
        TheCameraParameters.readFromXMLFile(TheIntrinsicFile);
        TheCameraParameters.resize(TheInputImage.size());
      }

      //Create gui
#ifdef SHOW_THRESHOLDED
      cv::namedWindow("thres",1);
#endif

      TheBoardDetector.setParams(TheBoardConfig,TheCameraParameters,TheMarkerSize);
      TheBoardDetector.getMarkerDetector().getThresholdParams( ThresParam1,ThresParam2);
      TheBoardDetector.getMarkerDetector().setCornerRefinementMethod(MarkerDetector::HARRIS);
      TheBoardDetector.set_repj_err_thres(1.5);

#ifdef SHOW_GUI
      cv::namedWindow("in",1);      
      iThresParam1=ThresParam1;
      iThresParam2=ThresParam2;
      cv::createTrackbar("ThresParam1", "in", &iThresParam1, 13, cvTackBarEvents);
      cv::createTrackbar("ThresParam2", "in", &iThresParam2, 13, cvTackBarEvents);
#endif
      
      char key = 0;
      int index = 0;

      // Set up an OSC recipient
      lo_address recipient = lo_address_new("192.168.0.111", "14040");
      
      //capture until press ESC or until the end of the video
      do
        {
          TheVideoCapturer.retrieve(TheInputImage);
          
          // Copy the image into the TheInputImageCopy while at the same time resizing it
          resize(TheInputImage, TheInputImageCopy, Size(), 0.5, 0.5, CV_INTER_AREA);
          
          index++; //number of images captured
          double tick = (double) getTickCount(); //for checking the speed
          
          // Detection of the board
          float probDetect=TheBoardDetector.detect(TheInputImage);

          // Check the speed 
          cout<<"Time detection="<< (((double) getTickCount() - tick) / getTickFrequency())*1000 <<" milliseconds"<<endl;

#ifdef SHOW_GUI
           // Print marker borders
           for (int i = 0; i < TheBoardDetector.getDetectedMarkers().size(); i++) {
             TheBoardDetector.getDetectedMarkers()[i].draw(TheInputImageCopy,Scalar(0,0,255),1);
           }
           
           // Initialize an indicator. It turns green if a board is detected.
           cv::circle(TheInputImageCopy, cv::Point(20,20), 15, cv::Scalar(0,0,255,0), -1);
#endif
          
          // Print board
          if (TheCameraParameters.isValid()) {
            if ( probDetect > 0.0020) {
#ifdef SHOW_GUI
              cv::circle(TheInputImageCopy, cv::Point(20,20), 15, cv::Scalar(0,255,0,0), -1);
              
              CvDrawingUtils::draw3dAxis(TheInputImageCopy, TheBoardDetector.getDetectedBoard(), TheCameraParameters);
              CvDrawingUtils::draw3dCube(TheInputImageCopy, TheBoardDetector.getDetectedBoard(), TheCameraParameters);
#endif
              // We can access Rvec and Tvec like so
              // cout << TheBoardDetector.getDetectedBoard().Rvec << endl;
              // cout << TheBoardDetector.getDetectedBoard().Tvec << endl;

              // Send an OSC packet to with the RT vectors
              lo_send(recipient,
                      "/camera",
                      "ffffff",
                      TheBoardDetector.getDetectedBoard().Rvec.at<float>(0,0),
                      TheBoardDetector.getDetectedBoard().Rvec.at<float>(1,0),
                      TheBoardDetector.getDetectedBoard().Rvec.at<float>(2,0),
                      TheBoardDetector.getDetectedBoard().Tvec.at<float>(0,0),
                      TheBoardDetector.getDetectedBoard().Tvec.at<float>(1,0),
                      TheBoardDetector.getDetectedBoard().Tvec.at<float>(2,0)
                      );
              printf("+\r");
            } else {
              printf("-\r");
            }
            
          }

#ifdef SHOW_GUI
          //show input with augmented information and  the thresholded image
          cv::imshow("in",TheInputImageCopy);
          
          key=cv::waitKey(waitTime);//wait for key to be pressed
          processKey(key);
#endif

#ifdef SHOW_THRESHOLDED
          cv::imshow("thres",TheBoardDetector.getMarkerDetector().getThresholdedImage());
#endif
        } while ( key!=27 && TheVideoCapturer.grab());


    } catch (std::exception &ex) {
      cout<<"Exception :"<<ex.what()<<endl;
    }

}
      
#ifdef SHOW_GUI
void cvTackBarEvents(int pos,void*)
{
  if (iThresParam1<3) iThresParam1=3;
  if (iThresParam1%2!=1) iThresParam1++;
  if (ThresParam2<1) ThresParam2=1;
  ThresParam1=iThresParam1;
  ThresParam2=iThresParam2;
  TheBoardDetector.getMarkerDetector().setThresholdParams(ThresParam1,ThresParam2);
  //recompute
  //Detection of the board
  float probDetect=TheBoardDetector.detect( TheInputImage);
  TheInputImage.copyTo(TheInputImageCopy);
  if (TheCameraParameters.isValid() && probDetect>0.2)
    aruco::CvDrawingUtils::draw3dAxis(TheInputImageCopy,TheBoardDetector.getDetectedBoard(),TheCameraParameters);

  cv::imshow("in",TheInputImageCopy);
  cv::imshow("thres",TheBoardDetector.getMarkerDetector().getThresholdedImage());
}
#endif 
