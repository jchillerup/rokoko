#include <iostream>
#include "stdio.h"
#include "opencv2/opencv.hpp"
#include "rokoko-common.c"
#include "lo/lo.h"
#include "sallycap.hpp"

using namespace std;
using namespace cv;

int iLowH = 38;
int iHighH = 98;
int iLowS = 82;
int iHighS = 255;
int iLowV = 64;
int iHighV = 255;
int contourAreaMin = 50;
int eyeThresh = 52;
int minGreen = 70;
float gor = 1.1;
int eyeRectX = 358;
int eyeRectY = 66;
int eyeRectH = 159;

std::string osc_address;
lo_address recipient;

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
  createTrackbar("eyeRectX", "Tunables", &eyeRectX, 500);
  createTrackbar("eyeRectY", "Tunables", &eyeRectY, 300);
  createTrackbar("eyeRectH", "Tunables", &eyeRectH, 200);
  //createTrackbar("greenOverRed", "Tunables", &gor, 5);
}

int main(int argc, char* argv[])
{
  rokoko_face cur_face;

  if (argc != 4 && argc != 5) {
	  cout << "Not enough commandline arguments given" << endl;
	  return EXIT_FAILURE;
  }

  string osc_recipient = argv[2];
  recipient = lo_address_new(osc_recipient.c_str(), "14040");
  osc_address = argv[3];

  if (argc == 5) {
    load_presets(argv[4]);
  }
  
  VideoCapture cap(atoi(argv[1])); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;

  spawnSettingsWindow();
  
  Mat edges;
  namedWindow("face",1);
  for(;;) {
    Mat frame, frame2;
    Rect right_eye_rect = Rect(eyeRectX, eyeRectY, 129, eyeRectH);
    Rect left_eye_rect = Rect(eyeRectX, eyeRectY+eyeRectH, 129, eyeRectH);

    cap >> frame; // get a new frame from camera
    if(frame.empty()) break;
      
    // Transpose the frame to get an upright view.
    //frame = frame2.t();

    // cvtColor(frame, edges, CV_BGR2GRAY);
      
    //findFacialMarkersGOR(frame, &cur_face);
    findFacialMarkersHSV(frame, &cur_face);
      
    cur_face.right_eye = findDarkestPoint(frame, right_eye_rect);
    cur_face.left_eye = findDarkestPoint(frame, left_eye_rect);

    dispatch_osc(&cur_face);
      
    IDContours(&cur_face, frame);
      
    imshow("face", frame);
    if(waitKey(30) >= 0) break;
  }

  if (argc == 5) {
    save_presets(argv[4]);
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

    cur_face->contours[contours_idx].center = center;
    //strcpy(cur_face->contours[contours_idx].label, "Test"); // TODO: Fix this

    contours_idx++;
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
    circle(frame, cur_face->contours[i].center, 3, Scalar(0, 0, 255), -1);
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
    circle(frame, cur_face->contours[i].center, 3, Scalar(0, 0, 255), -1);
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
  
  //imshow("eye", eye_thresholded);
  findContours( eye_thresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  largestContour = findLargestContour(contours);

  float size = 0;
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
  
    circle(frame, centerOffset, 1, Scalar(255, 255, 0));
    return centerOffset;
  }
}

void IDContours(rokoko_face* cur_face, cv::Mat frame) {
  std::vector<cv::Point> facePoints;
  std::vector<std::vector<cv::Point> > hullVector;
  
  for (int i = 0; i < cur_face->num_contours; i++) {
    facePoints.push_back(cur_face->contours[i].center);
  }

  if (facePoints.size() == 0) {
    return;
  }
  
  // Make a convex hull around all found contours.
  std::vector<cv::Point> hull;  
  convexHull(facePoints, hull, true, true); // TODO: Can probably work faster if last flag is false

  // TODO: hack. We need contours to be in an array so we create one and add
  // our convex hull to it.
  hullVector.push_back(hull);
  drawContours(frame, hullVector, -1, Scalar(0,255,0));

  // Get the center of the convex hull and mark it. We're using it to estimate
  // the positions of other markers
  cv::Point hullCenter = getContourCentroid(hull);
  circle(frame, hullCenter, 3, Scalar(0,255,0));
  
  // Find the point nearest to the center of the convex hull.
  // We assume this to be the tip of the nose.
  int minDist = 100000;
  int minIdx = -1;
  for (int i = 0; i < facePoints.size(); i++) {
    double distance = cv::norm( hullCenter - facePoints[i] );

    if (distance < minDist) {
      minDist =  distance;
      minIdx = i;
    }
  }

  if (minIdx != -1) {
	  circle(frame, facePoints[minIdx], 5, Scalar(0, 255, 0));
  }
}



void dispatch_osc(rokoko_face* cur_face) {
  //pretty_print_face(cur_face);
  
  lo_blob blob = lo_blob_new(sizeof(rokoko_face), cur_face);
  lo_send(recipient, osc_address.c_str(), "b", blob);
}

void load_presets(std::string filename) {
  FileStorage fs(filename, FileStorage::READ);

  if (!fs.isOpened()) {
    return;
  }

  iLowH = (int) fs["iLowH"];
  iHighH = (int) fs["iHighH"];
  iLowS = (int) fs["iLowS"];
  iHighS = (int) fs["iHighS"];
  iLowV = (int) fs["iLowV"];
  iHighV = (int) fs["iHighV"];
  contourAreaMin = (int) fs["contourAreaMin"];
  eyeThresh = (int) fs["eyeThresh"];
  minGreen = (int) fs["minGreen"];
  gor = (float) fs["gor"];
  eyeRectX = (int) fs["eyeRectX"];
  eyeRectY = (int) fs["eyeRectY"];
  eyeRectH = (int) fs["eyeRectH"];

  fs.release();
}

void save_presets(std::string filename) {
  FileStorage fs(filename, FileStorage::WRITE);

  if (!fs.isOpened()) {
    return;
  }

  fs << "iLowH" << iLowH;
  fs << "iHighH" << iHighH;
  fs << "iLowS" << iLowS;
  fs << "iHighS" << iHighS;
  fs << "iLowV" << iLowV;
  fs << "iHighV" << iHighV;
  fs << "contourAreaMin" << contourAreaMin;
  fs << "eyeThresh" << eyeThresh;
  fs << "minGreen" << minGreen;
  fs << "gor" << gor;
  fs << "eyeRectX" << eyeRectX;
  fs << "eyeRectY" << eyeRectY;
  fs << "eyeRectH" << eyeRectH;

  fs.release();
}

