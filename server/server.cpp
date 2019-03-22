/**
  Copyright 2017 by Satya Mallick ( Big Vision LLC )
http://www.learnopencv.com
 **/

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
	// Read image
	Mat img = imread(argv[1],CV_LOAD_IMAGE_COLOR);
	
	
	// Output image
	Mat imgOut = img.clone();

	imshow("Red Eyes", img);
	imshow("Red Eyes Removed", imgOut);
	waitKey(0);

}
