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
void findDarkestPoint(cv::Mat frame, cv::Rect region);

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
      Mat frame, frame2;
      Rect right_eye_rect = Rect(358, 66, 129, 159);
      Rect left_eye_rect = Rect(358, 226, 129, 159);
      cap >> frame; // get a new frame from camera

      // Transpose the frame to get an upright view.
      frame2 = frame.t();
      
      cvtColor(frame, edges, CV_BGR2GRAY);
      findFacialMarkers(frame, &cur_face);
      findDarkestPoint(frame, right_eye_rect);
      findDarkestPoint(frame, left_eye_rect);
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
void findDarkestPoint(cv::Mat frame, cv::Rect region) {
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
  
  cv::Mat eye_thresholded = eye_channels[2] > 25;

  /* Find contours in the thresholded image to look at. The first
     contour will be the eye area and be rather large. There will
     be one or more holes in that large contour which will be the
     eye or eye lids in case the eye is closed. We'll find the
     largest of the sub-contours within the eye and use the
     center of that to estimate the pupil position.
   */
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

  if (largestContourWithin != -1) {
    Point center = getContourCentroid(contours[largestContourWithin]);
    Point centerOffset = center + Point(region.x, region.y);
  
    circle(frame, centerOffset, 1, Scalar(255, 255, 0));
  
    imshow("eye r", eye_thresholded);
  }
  // Debug; draw the region that we're looking inside
  rectangle(frame, region, 0, 1);
}
