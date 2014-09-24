#include <iostream>
#include "opencv2/opencv.hpp"
#include "rokoko-common.c"

using namespace std;
using namespace cv;

int iLowH = 47;
int iHighH = 77;
int iLowS = 82;
int iHighS = 255;
int iLowV = 64;
int iHighV = 255;
int contourAreaMin = 100;

void findFacialMarkers(cv::Mat frame, rokoko_face* cur_face);

int main(int, char**)
{
  rokoko_face cur_face;
  
  VideoCapture cap("../samples/sally/1.wmv"); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;

  Mat edges;
  namedWindow("face",1);
  for(;;)
    {
      Mat frame;
      cap >> frame; // get a new frame from camera
      cvtColor(frame, edges, CV_BGR2GRAY);
      findFacialMarkers(frame, &cur_face);
      imshow("face", frame);
      if(waitKey(30) >= 0) break;
    }

  return 0;
}

void findFacialMarkers(cv::Mat frame, rokoko_face* cur_face) {
  vector<vector<cv::Point> > contours;
  vector<cv::Vec4i> hierarchy;
  int contours_idx = 0;

  cv::Mat imgHSV;
  cv::cvtColor(frame, imgHSV, cv::COLOR_RGB2HSV); //Convert the captured frame from RGB to HSV

  cv::Mat imgThresholded;
  cv::inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

  // TODO: Apply erosion/dilation to imgThresholded?


  cv::findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

  imshow("thrsholded", imgThresholded);
  
  for (int i = 0; i < contours.size(); i++) {
    if (contourArea(contours[i]) < contourAreaMin || contours_idx == MAX_CONTOURS) continue;

    Rect bound = boundingRect(contours[i]);
    Point center = Point( bound.x + (bound.width / 2), bound.y + (bound.height / 2));

    // We don't need to store imgThresholded.cols and .rows / 2 because <3 compilers.
    //cout << "(" << center.x - (imgThresholded.cols/2) << ", " << center.y - (imgThresholded.rows/2) << ")" << endl;

    circle(frame, center, 3, Scalar(0, 0, 255), -1);

    cur_face->contours[contours_idx++] = center;
  }

  
  cur_face->num_contours = contours_idx;
}
