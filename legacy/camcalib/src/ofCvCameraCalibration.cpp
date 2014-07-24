/*
*  camera calibration
*  stefanix - helloworld@stefnix.net
*/

#include "ofCvCameraCalibration.h"



void ofCvCameraCalibration::allocate( CvSize _csize, int _nCornersX, int _nCornersY ) {
    csize = _csize;
    nCornersX = _nCornersX;
    nCornersY = _nCornersY;
}


bool ofCvCameraCalibration::addImage( IplImage* img ) {

    vector<ofPoint2f> points;
    IplImage* bw = cvCreateImage( csize, IPL_DEPTH_8U, 1 );
    cvCvtColor( img, bw, CV_RGB2GRAY );
    
    bool bFound = findCorners( bw, points );
    if( bFound ) {
        //image
        IplImage* color = cvCreateImage( csize, IPL_DEPTH_8U, 3 );
        cvCopy( img, color );
        drawCircles( color, points );
        colorImages.push_back( color );
        
        //screen points
        for( int i=0; i<(nCornersX*nCornersY); ++i ) {
            screenPoints.push_back( ofPoint2f(points[i].x, points[i].y) );
        }
        
        //world points based on a 8"x8" chess board; assumes z == 0 plane
        for( int x=0; x<nCornersX; ++x ) {
            for( int y=0; y<nCornersY; ++y ) {
                worldPoints.push_back( ofPoint3f(x,y,0) );
            }
        }
        cvReleaseImage( &bw );
        cout << "Image# " << colorImages.size() << endl;
        return true;
    } else {
        cvReleaseImage( &bw );
        cout << "Finding corners FAILED!" << endl;
        return false;
    }
}




void ofCvCameraCalibration::calibrate() {
    calibrateCamera( colorImages.size(),
                     screenPoints,
                     worldPoints,
                     distortionCoeffs,
                     camIntrinsics,
                     transVectors,
                     rotMatrices );

}




void ofCvCameraCalibration::undistort() {
	/*
    CvPoint2D32f* backprojPts2d = 0;
    backprojPts2d = ConvertWorldToPixel( corners3d, 
                                         numImages, 
                                         cornersFound, 
                                         camera_matrix, 
                                         translation_vectors, 
                                         rotation_matrices );
    */
    
	//convert
    CvVect32f  dist = new float[4];
	CvMatr32f  camM = new float[9];

    for( int i=0; i<4; ++i) {
        dist[i] = distortionCoeffs[i];
    }        
    
    for( int i=0; i<9; ++i) {
        camM[i] = camIntrinsics[i];
    }    
    
	for (int i=0; i<colorImages.size(); i++) {
        undistortedImg.push_back( cvCreateImage(csize, IPL_DEPTH_8U, 3) );                 
        cvUnDistortOnce(colorImages[i], undistortedImg[i], camM, dist, 1);
        
        /*
        drawCircles( undistortedImg[i], points );
		DrawCircles( image, &backprojPts2d[iImg*cornersFound[iImg]], 
                    cornersFound[iImg], "Image", 1 );
		DrawCircles( undistort, &backprojPts2d[iImg*cornersFound[iImg]], 
                     cornersFound[iImg], "Undistorted", 1 );
        */
	}
    
    delete dist;
    delete camM;
}





bool ofCvCameraCalibration::findCorners( const IplImage* bw,
                               vector<ofPoint2f>& points
                             ) const {
    IplImage* tempImg = cvCreateImage( csize, IPL_DEPTH_8U, 1 );
    
	CvPoint2D32f* p = new CvPoint2D32f[nCornersX*nCornersY];
    int nCorners;
    
    int bPerfect = cvFindChessBoardCornerGuesses( bw, 
                                                  tempImg, 
                                                  NULL, 
                                                  cvSize(nCornersX, nCornersY), 
                                                  p, 
                                                  &nCorners );
    cvFindCornerSubPix( bw, 
                        p, 
                        nCorners, 
                        cvSize(5,5), cvSize(-1,-1), 
                        cvTermCriteria(CV_TERMCRIT_ITER, 100, 0.1) );

    //convert back
    for( int i=0; i<(nCornersX*nCornersY); ++i ) {
        points.push_back( ofPoint2f(p[i].x,p[i].y) );
    }    

    cvReleaseImage( &tempImg );
    delete p;	    

    if( !bPerfect ) {
        cout << "Did not find expected number of points...\n";
        return false;
    } else {
        return true;
    }
}






void ofCvCameraCalibration::calibrateCamera( const int                 nImages,
                                   const vector<ofPoint2f>&  _screenPoints,
                                   const vector<ofPoint3f>&  _worldPoints,
                                   ofVec4f&                  _distortionCoeffs,
                                   ofMatrix3x3&              _camIntrinsics,
                                   vector<ofVec3f>&          _transVectors,
                                   vector<ofMatrix3x3>&      _rotMatrices
                                 ) const {
    //assume all points have been for all images
    int* nCorners = new int[nImages];
    for( int i=0; i<nImages; ++i ) {
        nCorners[i] = nCornersX*nCornersY;
    }
    
	CvPoint2D32f* screenP = new CvPoint2D32f[nImages*nCornersX*nCornersY];
    for( int i=0; i<nImages*nCornersX*nCornersY; ++i ) {
        screenP[i].x = _screenPoints[i].x;
        screenP[i].y = _screenPoints[i].y;
    }    
    
	CvPoint3D32f* worldP = new CvPoint3D32f[nImages*nCornersX*nCornersY];
    for( int i=0; i<nImages*nCornersX*nCornersY; ++i ) {
        worldP[i].x = _worldPoints[i].x;
        worldP[i].y = _worldPoints[i].y;
        worldP[i].z = _worldPoints[i].z;
    }
    
	CvVect32f  distortion = new float[4];
	CvMatr32f  camera_matrix = new float[9];
	CvVect32f  translation_vectors = new float[3*nImages];
    CvMatr32f  rotation_matrices = new float[9*nImages];
    
	cvCalibrateCamera( nImages,            //Number of the images.
                       nCorners,             //Array of the number of points in each image.
                       csize,                //Size of the image.
                       screenP,              //Pointer 2D points in screen space. 
                       worldP,               //Pointer 3D points in real space
                       distortion,           //output: 4 distortion coefficients
                       camera_matrix,        //output: intrinsic camera matrix
                       translation_vectors,  //output: Array of translations vectors
		               rotation_matrices,    //output: Array of rotation matrices
                       0 );                  //intrisic guess needed

    delete nCorners;
    delete screenP;
    delete worldP;
    
    
    
	PrintIntrinsics( distortion, camera_matrix );
    
    //convert coefficients
    _distortionCoeffs.set( distortion[0], distortion[1], 
                           distortion[2], distortion[3] );   
    
    //convert camera matrix
    for( int i=0; i<9; ++i) {
        _camIntrinsics[i] = camera_matrix[i];
    }
    
    //convert translation vectors
    for( int iImg=0; iImg<nImages; ++iImg ) {
        _transVectors.push_back( ofVec3f(translation_vectors[iImg*3], 
                                         translation_vectors[iImg*3+1], 
                                         translation_vectors[iImg*3+2]) );
    }

    //convert rotation matrices
    for( int iImg=0; iImg<nImages; ++iImg ) {
        _rotMatrices.push_back( ofMatrix3x3(rotation_matrices[iImg*9], 
                                           rotation_matrices[iImg*9+1], 
                                           rotation_matrices[iImg*9+2],
                                           rotation_matrices[iImg*9+3],
                                           rotation_matrices[iImg*9+4],
                                           rotation_matrices[iImg*9+5],
                                           rotation_matrices[iImg*9+6],
                                           rotation_matrices[iImg*9+7],
                                           rotation_matrices[iImg*9+8]) );
    }    
    
    delete distortion;
    delete camera_matrix;
    delete translation_vectors;
    delete rotation_matrices;
}







void ofCvCameraCalibration::drawCircles( IplImage* img, vector<ofPoint2f>& points ) {
    CvPoint pt;
	for( int i= 0; i<points.size(); ++i ) {
		int color = (int)( (float)i/points.size()*255.0 );
		pt.x = (int)points[i].x;
		pt.y = (int)points[i].y;
		cvCircle( img, pt, 3, CV_RGB(255-color,0,color), CV_FILLED );
	}
}







/* 
 * Backprojection from World to Image coordinates
 */
CvPoint2D32f* ofCvCameraCalibration::ConvertWorldToPixel( CvPoint3D32f *pts3d, 
                                                int numImages, 
                                                int *numPts, 
                                                CvMatr32f cam, 
                                                CvVect32f t, 
                                                CvMatr32f r) {
	int i, j, k;
	CvPoint2D32f *pts2d    = new CvPoint2D32f[numImages * 49];

	CvMat *C		= cvCreateMat(3, 3, CV_32FC1);
	CvMat *point3D  = cvCreateMat(3, 1, CV_32FC1);
	CvMat *R        = cvCreateMat(3, 3, CV_32FC1);
	CvMat *T        = cvCreateMat(3, 1, CV_32FC1);

	CvPoint3D32f *pts3dCur = pts3d;
	CvPoint2D32f *pts2dCur = pts2d;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
		{
			CV_MAT_ELEM(*C, float, i, j) = cam[3 * i + j];
		}

	for (k = 0; k < numImages; k++) {
		for (j = 0; j < 9; j++) {
			if (j < 3) CV_MAT_ELEM(*T, float, j, 0) = t[k*3 + j];
			CV_MAT_ELEM(*R, float, j / 3, j%3) = r[k*9 + j];
		}

		for (i = 0; i < numPts[k]; i++)
		{	
			CV_MAT_ELEM(*point3D, float, 0, 0) = pts3dCur[i].x;
			CV_MAT_ELEM(*point3D, float, 1, 0) = pts3dCur[i].y;
			CV_MAT_ELEM(*point3D, float, 2, 0) = pts3dCur[i].z;


			cvMatMulAdd(R, point3D, T, point3D); //rot and translate
			cvMatMul(C, point3D, point3D);     //camera 
			
			pts2dCur[i].x = CV_MAT_ELEM(*point3D, float, 0, 0) / CV_MAT_ELEM(*point3D, float, 2, 0);
			pts2dCur[i].y = CV_MAT_ELEM(*point3D, float, 1, 0) / CV_MAT_ELEM(*point3D, float, 2, 0);
		}
		pts3dCur += numPts[k];
		pts2dCur += numPts[k];
	}
	
	cvReleaseMat(&point3D);
	cvReleaseMat(&T);
	cvReleaseMat(&R);

	return pts2d;
}




void ofCvCameraCalibration::printIntrinsics( const CvVect32f& distortion, 
                                   const CvMatr32f& camera_matrix) const {
    if (distortion) {
		cout << "Distortion Coefficients:" << endl;
        for( int i=0; i<4; ++i) {
            cout << distortion[0] << ", ";
        }
        cout << endl;
    }

	if (camera_matrix)	{
		printf("Camera Matrix:\n");
        int m = 0;
		for( int j=0; j<3; ++j ) {
			for( int i=0; i<3; ++i ) {
				cout << camera_matrix[m++] << ", ";
			}
			cout << endl;
		}
	}
}



void ofCvCameraCalibration::PrintIntrinsics( const CvVect32f& _distortion, 
                                   const CvMatr32f& _camera_matrix) const {
	int i;
	if (_distortion)
	{
		printf("Distortion Coefficients:\n");
		printf("%4.10f ", _distortion[0]);
		printf("%4.10f ", _distortion[1]);
		printf("%4.10f ", _distortion[2]);
		printf("%4.10f ", _distortion[3]);
		printf("\n");
	}

	if (_camera_matrix)
	{
		printf("Camera Matrix:\n");
		PrintMatrix(_camera_matrix, 3, 3);
	}
}

void ofCvCameraCalibration::PrintMatrix( const CvMatr32f& matrix, 
                               unsigned int rows, unsigned int cols) const {
	int m, i, j;
	if (matrix)
	{
		m = 0;
		for (j = 0; j < rows; j++)
		{
			for (i = 0; i < cols; i++)
			{
				printf("%4.10f ",matrix[m++]);
			}
			printf("\n");
		}
	}
}


