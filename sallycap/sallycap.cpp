#include <iostream>
#include "opencv2/opencv.hpp"
#include "rokoko-common.c"
#include "lo/lo.h"
#include "sallycap.hpp"
#include "stdio.h"

using namespace std;
using namespace cv;

int iLowH = 47;
int iHighH = 77;
int iLowS = 82;
int iHighS = 255;
int iLowV = 64;
int iHighV = 255;
int contourAreaMin = 160;
int eyeThresh = 52;
int minGreen = 70;
float gor = 1.2;


void spawnSettingsWindow() {
  cv::namedWindow("Tunables", CV_WINDOW_NORMAL);
  cv::moveWindow("Tunables", 400, 100);
  
  // Add trackbars for the HSV settings
  createTrackbar("LowH",  "Tunables", &iLowH, 179); //Hue (0 - 179)
  createTrackbar("HighH", "Tunables", &iHighH, 179);
  createTrackbar("LowS",  "Tunables", &iLowS, 255); //Saturation (0 - 255)
  createTrackbar("HighS", "Tunables", &iHighS, 255);
  createTrackbar("LowV",  "Tunables", &iLowV, 255); //Value (0 - 255)
  createTrackbar("HighV", "Tunables", &iHighV, 255);
  createTrackbar("ContourAreaMin", "Tunables", &contourAreaMin, 1000);
  createTrackbar("eyeThresh", "Tunables", &eyeThresh, 255);
  createTrackbar("minGreen", "Tunables", &minGreen, 255);
  //createTrackbar("greenOverRed", "Tunables", &gor, 5);
}

int main(int, char**)
{
  rokoko_face cur_face;
  
  VideoCapture cap("../samples/sally/1.wmv"); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;

  spawnSettingsWindow();
  
  Mat edges;
  namedWindow("face",1);
  for(;;)
    {
      Mat frame, frame2;
      Rect right_eye_rect = Rect(358, 66, 129, 159);
      Rect left_eye_rect = Rect(358, 226, 129, 159);
      cap >> frame; // get a new frame from camera

      // Transpose the frame to get an upright view.
      frame2 = frame.t();
      
      cvtColor(frame, edges, CV_BGR2GRAY);
      
      findFacialMarkersGOR(frame, &cur_face);
      //findFacialMarkersHSV(frame, &cur_face);
      
      findDarkestPoint(frame, right_eye_rect);
      findDarkestPoint(frame, left_eye_rect);

      IDContours(&cur_face, frame);
      
      imshow("face", frame);
      if(waitKey(30) >= 0) break;
    }

  return 0;
}

void findAndAddContoursToFace(cv::Mat imgThresholded, rokoko_face* cur_face) {
  vector<vector<cv::Point> > contours;
  vector<cv::Vec4i> hierarchy;
  int contours_idx = 0;
  
  cv::findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));
  
  for (int i = 0; i < contours.size(); i++) {
    if (contourArea(contours[i]) < contourAreaMin || contours_idx == MAX_CONTOURS) continue;

    Rect bound = boundingRect(contours[i]);
    Point center = Point( bound.x + (bound.width / 2), bound.y + (bound.height / 2));

    // We don't need to store imgThresholded.cols and .rows / 2 because <3 compilers.
    //cout << "(" << center.x - (imgThresholded.cols/2) << ", " << center.y - (imgThresholded.rows/2) << ")" << endl;

    cur_face->contours[contours_idx++] = center;
  }
  
  cur_face->num_contours = contours_idx;
}

void findFacialMarkersHSV(cv::Mat frame, rokoko_face* cur_face) {

  cv::Mat imgHSV;
  cv::cvtColor(frame, imgHSV, cv::COLOR_RGB2HSV); //Convert the captured frame from RGB to HSV

  cv::Mat imgThresholded;
  cv::inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

  imshow("Thresholded, HSV", imgThresholded);

  findAndAddContoursToFace(imgThresholded, cur_face);
  
  for (int i = 0; i < cur_face->num_contours; i++) {
    circle(frame, cur_face->contours[i], 3, Scalar(0, 0, 255), -1);
  }
}

void findFacialMarkersGOR(cv::Mat frame, rokoko_face* cur_face) {
  std::vector<cv::Mat> rgbChannels(3);
  cv::split(frame, rgbChannels);
  cv::Mat greens;



  cv::bitwise_and(rgbChannels[1] > minGreen, rgbChannels[1] > (rgbChannels[2] * gor), greens);
  cv::bitwise_and(greens, rgbChannels[1] > (rgbChannels[0] * gor), greens);

  imshow("Thresholded, GOR", greens);

  findAndAddContoursToFace(greens, cur_face);
  
  for (int i = 0; i < cur_face->num_contours; i++) {
    circle(frame, cur_face->contours[i], 3, Scalar(0, 0, 255), -1);
  }
}

int findLargestContour(vector<vector<Point> > contours) {
  float largestArea = -1;
  vector<Point> largestContour;
  int largestContourIdx = -1;
  
  for( int i = 0; i< contours.size() ; i++) {
    float area = contourArea(contours[i]);
    if (area > largestArea) {
      largestContour = contours[i];
      largestArea = area;
      largestContourIdx = i;
    }
  }

  return largestContourIdx;
}

Point getContourCentroid(vector<Point> contour) {
  Moments m = moments(contour);
  return Point((int)(m.m10/m.m00), (int)(m.m01/m.m00));
}

// We assume the pupil is going to be in the darkest point of a rectangle containing the eye area.
cv::Point findDarkestPoint(cv::Mat frame, cv::Rect region) {
  cv::Mat eye = frame(region);
  cv::Mat eye_channels[3];
  vector<vector<Point> > contours;
  int largestContour;
  vector<Vec4i> hierarchy;
  
  split(eye, eye_channels);
  /*
  imshow("eye b", eye_channels[0]);
  imshow("eye g", eye_channels[1]);
  */
  
  cv::Mat eye_thresholded;
  inRange(eye_channels[2], eyeThresh, 255, eye_thresholded);

  /* Find contours in the thresholded image to look at. The first
     contour will be the eye area and be rather large. There will
     be one or more holes in that large contour which will be the
     eye or eye lids in case the eye is closed. We'll find the
     largest of the sub-contours within the eye and use the
     center of that to estimate the pupil position.
   */
  
  imshow("eye"+ region.y, eye_thresholded);
  findContours( eye_thresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  largestContour = findLargestContour(contours);

  float size;
  int largestContourWithin = -1;
  // Find the largest of the contours that have the largestContour as parent.
  for (int i=0; i < hierarchy.size(); i++) {
    if (hierarchy[i][3] == largestContour) {
      if (contourArea(contours[i]) > size) {
        largestContourWithin = i;
        size = contourArea(contours[i]);
      }
    }
  }
  // Debug; draw the region that we're looking inside
  rectangle(frame, region, 0, 1);
  
  if (largestContourWithin != -1) {
    Point center = getContourCentroid(contours[largestContourWithin]);
    Point centerOffset = center + Point(region.x, region.y);
  
    //circle(frame, centerOffset, 1, Scalar(255, 255, 0));
    return centerOffset;
  }
}

int findCenterContour(rokoko_face* cur_face) {


  
}

void IDContours(rokoko_face* cur_face, cv::Mat frame) {
  std::vector<cv::Point> facePoints;
  std::vector<std::vector<cv::Point> > hullVector;
  facePoints.assign(cur_face->contours, cur_face->contours + cur_face->num_contours);

  // Make a convex hull around all found contours.
  std::vector<cv::Point> hull;  
  convexHull(facePoints, hull, true, true); // TODO: Can probably work faster if last flag is false

  // TODO: hack. We need contours to be in an array so we create one and add
  // our convex hull to it.
  hullVector.push_back(hull);
  
  drawContours(frame, hullVector, -1, Scalar(0,255,0));
  
  cv::Point hullCenter = getContourCentroid(hull);

  circle(frame, hullCenter, 3, Scalar(0,255,0));
  
  // Find the point nearest to the center of the convex hull.
  // We assume this to be the tip of the nose.
  //int center = findCenterContour(cur_face);

  int minDist = 100000;
  int minIdx = -1;
  for (int i = 0; i < facePoints.size(); i++) {
    double distance = cv::norm( hullCenter - facePoints[i] );

    if (distance < minDist) {
      minDist =  distance;
      minIdx = i;
    }
  }

  circle(frame, facePoints[minIdx], 5, Scalar(0,255,0));
  
}
