void spawnSettingsWindow();
void findAndAddContoursToFace(cv::Mat imgThresholded, rokoko_face* cur_face);
void findFacialMarkersHSV(cv::Mat frame, rokoko_face* cur_face);
void findFacialMarkersGOR(cv::Mat frame, rokoko_face* cur_face);
int findLargestContour(std::vector<std::vector<cv::Point> > contours);
cv::Point getContourCentroid(std::vector<cv::Point> contour);
cv::Point findDarkestPoint(cv::Mat frame, cv::Rect region);
int findCenterContour(rokoko_face* cur_face);
void IDContours(rokoko_face* cur_face, cv::Mat frame);
