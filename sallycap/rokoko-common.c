/** Constants **/
#define MAX_CONTOURS 21 /*;*/

typedef struct rokoko_contour {
  cv::Point center;
  //char label[20];
};

/** typedefs **/
typedef struct rokoko_face {
  int num_contours;
  rokoko_contour contours[MAX_CONTOURS];
  cv::Point left_eye;
  cv::Point right_eye;
} rokoko_face;

void pretty_print_face(rokoko_face* cur_face) {
  printf("Left pupil: (%d, %d)\n", cur_face->left_eye.x, cur_face->left_eye.y);
  printf("Right pupil: (%d, %d)\n", cur_face->right_eye.x, cur_face->right_eye.y);
  
  printf("Number of contours: %d\n", cur_face->num_contours);
  
  for (int i = 0; i < cur_face->num_contours; i++) {
    printf(" - C#%d\t%d\t%d\t\n", i, cur_face->contours[i].center.x, cur_face->contours[i].center.y /*, cur_face->contours[i].label*/);
  }
  printf("\n");
}
