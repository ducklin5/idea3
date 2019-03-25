#include <opencv2/opencv.hpp>
#include <algorithm>
#include <set>
#include <map>


using namespace std;
using namespace cv;

Mat threshold (Mat img, int radius) {
	Mat outImg(img.size(), CV_8UC1);
	// for each pixel
	for (int y = 0; y < img.rows; y++){
		for (int x = 0; x < img.cols; x++){
			//cout << "( " << x << ", " << y << ") "; 
			//cout << " out of : ";
			//cout << "( " << img.cols << ", " << img.rows << ")\n"; 
			// get the average value of surrounding pixels
			float avg = 0;
			int count = 0;

			// loop through surrounding pixels
			for(int j = -radius; j <= radius; j++){
				if(y+j < 0 or y+j >= img.rows) continue; // next if the neighbor doesnt exit (y)
				for(int i = -radius; i <= radius; i++){
					if(x+i < 0 or x+i >= img.rows) continue; // next if the neighbor doesnt exit (x)

					avg += img.at<uchar>(y+j,x+i);
					count ++;
				}
			}
			avg /= count;

			outImg.at<uchar>(y,x) = (img.at<uchar>(y, x) < avg * 0.9) * 255;

		}
	}

	return outImg;
}


Mat blobExtract( Mat binImg, map<int,int>& stats){
	Mat blobImg(binImg.size(), CV_32SC1, Scalar(0));
	int label = 0;
	map <int,set<int>> labelEqvs;
	
	// first pass
	for (int y = 0; y < binImg.rows; y++){
		for (int x = 0; x < binImg.cols; x++){
			uchar& currentPx = binImg.at<uchar>(y,x);
			int& currentLabel = blobImg.at<int>(y,x);

			if( currentPx >  0 ){
				// intialize variable to store top and left data (-1 means never set)
				int top = -1;
				int left = -1;
				// if the top/left exists, then get its value from binImg
				if ( y > 0 ) top = binImg.at<uchar>(y-1, x);
				if ( x > 0 ) left = binImg.at<uchar>(y, x-1);
				// if they both are not foreground pixels
				if (top <= 0 and left <= 0) {
					label+= 1;
					currentLabel = label;
					// otherwise at least one of them is foreground
				} else {
					// if the top is foreground, then set the current label to its label
					int toplbl = 0;
					int leftlbl = 0;
					if ( top > 0 ) {
						toplbl = blobImg.at<int>(y-1, x);
						currentLabel = toplbl;
					}
					if ( left > 0 ){
						leftlbl = blobImg.at<int>(y, x-1);
						currentLabel = blobImg.at<int>(y, x-1);
					}
					// if they both are foreground
					if ( top > 0 and left > 0 and toplbl != leftlbl ){
						// set the label to the lowest one and record the conflict
						currentLabel = ( toplbl < leftlbl ? toplbl: leftlbl);
						labelEqvs[toplbl].insert(leftlbl);
						labelEqvs[leftlbl].insert(toplbl);
					}
				}
			}
			//cout << "label: " << label << endl;
		}
	}
/*
	for (auto const& eqv : labelEqvs) {
		cout << eqv.first << ':';
		for (auto const& lbl : eqv.second){
			cout << lbl << ", ";
		}
		cout << std::endl ;
	}
*/
	map <int,int> simpleEqvs;
	
	for (auto const& eqv: labelEqvs){
		int crnt = eqv.first;
		while (labelEqvs[crnt].size() > 0){
			if(*labelEqvs[crnt].begin() < crnt){
				crnt = *labelEqvs[crnt].begin();
			} else {
				break;
			}
		}
		simpleEqvs[eqv.first] = crnt;
	}
/*
	for (auto const& eqv : simpleEqvs) {
		cout << eqv.first << " : ";
		cout << eqv.second << '\n';
	}
*/
	// second pass
	for (int y = 0; y < blobImg.rows; y++){
		for (int x = 0; x < blobImg.cols; x++){
			int& currentLabel = blobImg.at<int>(y,x);
			// get the first element of the set/ the lowest equivalent
			currentLabel = simpleEqvs[currentLabel];
			if (stats.count(currentLabel)==0) {
				stats[currentLabel] = 1;
			} else {
				stats[currentLabel]++;
			}
		}
	}
/*
	for (auto const& eqv : stats) {
		cout << eqv.first << " : ";
		cout << eqv.second << '\n';
	}
*/
	cout << stats.size() << " labels\n";;
	return blobImg;
}

template <typename fType, typename sType>
bool comparePair2 (const pair<fType, sType>& p1, const pair<fType, sType>& p2){
	return p1.second < p2.second;
}

bool fits (int num, set<int> vec, int thresh ){
	bool pass = true;
	for( auto elem : vec )
		if (abs (num - elem) < thresh ) { pass = false; break; }

	return pass;
}

int main(int argc, char** argv )
{
	// Read image
	Mat orig = imread(argv[1], IMREAD_COLOR );
	
	Mat gray;
	cvtColor(orig, gray, CV_RGB2GRAY);

	// radius based thersholding
	Mat black = threshold(gray, 5);

	// blob extraction - https://en.wikipedia.org/wiki/Connected-component_labeling
	// graph theory
	map<int, int> stats;
	Mat blobs = blobExtract(black, stats);
	
	// find the blob label with the most elements
	//https://stackoverflow.com/questions/30611709/find-element-with-max-value-from-stdmap
	stats[0] = 0;
	auto maxlbl_count = std::max_element(stats.begin(), stats.end(), comparePair2<int,int>);
	int maxlbl = maxlbl_count -> first;
	cout << "maxlbl: " << maxlbl << endl;
	
	// isolate the largest blob
	// and calculate the distances from the img corners to the largest blob pixels

	// this is where the edit image will go
	Mat majorBlob(blobs.size(), CV_8UC1, Scalar(0));
	// save the corners of the image
	vector<Point2f> imageConers = {
		Point(0,0),
		Point(blobs.cols, 0),
		Point(blobs.cols, blobs.rows),
		Point(0, blobs.rows)
	};
	// create 4 matrices to hold the distances for each pixel
	Mat dists[4];
		for (int i=0; i<4; i++)
			dists[i] = Mat(blobs.size(), CV_32SC1, Scalar(0));
	

	for (int y = 0; y < blobs.rows; y++){
		for (int x = 0; x < blobs.cols; x++){
			int& current = blobs.at<int>(y,x);
			uchar& mcurrent = majorBlob.at<uchar>(y,x);
			if (current == maxlbl){
				mcurrent = 1;
				for(int i=0; i<4; i++){
					dists[i].at<int>(y,x) = abs(y - imageConers[i].y) + abs(x - imageConers[i].x);
				}
			} else {
				mcurrent = 0;
			}
		}
	}
	
//	for(int i = 0; i < 4; i++){
//		Mat test;
//		dists[i].convertTo(test, CV_8UC1, 0.2);
//		imshow(to_string(i), test);
//	}

	// find the minimum for each corner
	Point sdkCorners[4];
	for(int i = 0; i < 4; i++){
		minMaxLoc(dists[i], NULL, NULL, &sdkCorners[i], NULL, majorBlob);
	}
	
	vector<Point2f> sdkCornersF (4);
	for(int i = 0; i < 4; i++)
		sdkCornersF[i] = sdkCorners[i];
	
	Size pSize(240,240);
	Mat puzzle = Mat( pSize, CV_32FC1, Scalar(0));

	imageConers = {
		Point2f(0,0),
		Point2f(pSize.width, 0),
		Point2f(pSize.height, pSize.width),
		Point2f(0, pSize.height)
	};

	Mat HTranform = findHomography(sdkCornersF, imageConers,  RANSAC);
	
	warpPerspective(gray, puzzle, HTranform, pSize);
	imshow("puzzle", puzzle);
	 
	cv::Mat puzzleC;
	cv::Canny(puzzle,puzzleC,30,20);
	
	Mat puzzleL = Mat( pSize, CV_32FC1, Scalar(0));

    vector<Vec2f> lines;
	HoughLines(puzzleC, lines, 1, CV_PI/180, 80, 0, 0 );
	
	set<int> xs = {0, pSize.width-1};
	set<int> ys = {0, pSize.height-1};
	for( size_t i = 0; i < lines.size(); i++ ){
		float rho = lines[i][0], theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		int x0 = a*rho, y0 = b*rho;
		
		int gap = 15;
		int degThresh = 2;
		theta  = theta * 180/CV_PI;

		if( (theta < degThresh or 180 - theta < degThresh )and fits(x0, xs, gap)) {
			xs.insert(x0);
		}
		
		if( abs(theta - 90) < degThresh and fits(y0, ys, gap)) {
			ys.insert(y0);
		}

//		if (draw){
//			Point pt1, pt2;
//			pt1.x = cvRound(x0 + 1000*(-b));
//			pt1.y = cvRound(y0 + 1000*(a));
//			pt2.x = cvRound(x0 - 1000*(-b));
//			pt2.y = cvRound(y0 - 1000*(a));
//			line( puzzleL, pt1, pt2, 255, 1);
//		}
	}
	imshow("puzzleC", puzzleC);
	//imshow("puzzleL", puzzleL);
	vector<int> xv(xs.begin(), xs.end());
	vector<int> yv(ys.begin(), ys.end());
	
	int sdkCols = yv.size()-1;
	int sdkRows = xv.size()-1;
	cout << "sdkCols: " << sdkCols << "\n";
	cout << "sdkRows: " << sdkRows << "\n";

	Mat pBoxes[sdkCols][sdkRows];

	for(int y = 0; y<sdkCols; y++){
		for(int x = 0; x<sdkCols; x++){
			pBoxes[y][x] = puzzle(Rect(xv[x], yv[y], xv[x+1]-xv[x], yv[y+1]-yv[y]));
			imshow(to_string(y*sdkCols+x), pBoxes[y][x]);
		}
	}

	waitKey(0);
	exit(0);
}
