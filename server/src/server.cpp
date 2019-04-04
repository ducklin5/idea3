#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
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
			// update the number of pixes in this label
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
	   
	cout << stats.size() << " labels\n";
	*/
	return blobImg;
}

template <typename fType, typename sType>
bool compare2nd (const pair<fType, sType>& p1, const pair<fType, sType>& p2){
	return p1.second < p2.second;
}

bool fits (int num, set<int> vec, int thresh ){
	bool pass = true;
	for( auto elem : vec )
		if (abs (num - elem) < thresh ) { pass = false; break; }

	return pass;
}

vector<Point2f> getCorners(Size pSize){
	vector<Point2f> corners = {
		Point2f(0,0),
		Point2f(pSize.width, 0),
		Point2f(pSize.height, pSize.width),
		Point2f(0, pSize.height)
	};
	return corners;
}

vector<Point2f> getSudokuCorners(Mat blobs, int puzzlelbl){
	// isolate/mask the blob of label `puzzlelbl`
	// and calculate the distances from each img corner to each mask pixel
	
	// save the corners of the original image
	vector<Point2f> imageConers = getCorners(blobs.size());

	// this is where the mask image will go
	Mat puzzleMask(blobs.size(), CV_8UC1, Scalar(0));

	// create 4 matrices to hold the distances for each pixel
	Mat dists[4];
	for (int i=0; i<4; i++) dists[i] = Mat(blobs.size(), CV_32SC1, Scalar(0));

	// for each pixel
	for (int y = 0; y < blobs.rows; y++){ for (int x = 0; x < blobs.cols; x++){
		// get its label (ref)
		int& current = blobs.at<int>(y,x);
		// get it mask value (ref)
		uchar& mcurrent = puzzleMask.at<uchar>(y,x);
		
		if (current == puzzlelbl){
			mcurrent = 1; // we care about it
			// get its distance from each corner
			for(int i=0; i<4; i++)
				dists[i].at<int>(y,x) = abs(y - imageConers[i].y) + abs(x - imageConers[i].x);
		} else {
			mcurrent = 0; // we dont care about it
		}
	} }

	//	for(int i = 0; i < 4; i++){
	//		Mat test;
	//		dists[i].convertTo(test, CV_8UC1, 0.2);
	//		imshow(to_string(i), test);
	//	}
	
	// find the minimum for each corner
	Point sdkCorners[4];
	for(int i = 0; i < 4; i++){
		minMaxLoc(dists[i], NULL, NULL, &sdkCorners[i], NULL, puzzleMask);
	}

	// convert the point type
	vector<Point2f> sdkCornersF (4);
	for(int i = 0; i < 4; i++) sdkCornersF[i] = sdkCorners[i];
	return sdkCornersF;
}

void getHVLinePos(Mat image, vector<int>& vPos, vector<int>& hPos){
	cv::Mat imageC;
	cv::Canny(image,imageC,30,20);

	Mat imageL = Mat(image.size(), CV_32FC1, Scalar(0));

	vector<Vec2f> lines;
	HoughLines(imageC, lines, 1, CV_PI/180, 80, 0, 0 );

	set<int> xs = {0, image.cols-1};
	set<int> ys = {0, image.rows-1};
	for( size_t i = 0; i < lines.size(); i++ ){
		float rho = lines[i][0], theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		int x0 = a*rho, y0 = b*rho;

		float gap = 0.05*image.cols;
		int degThresh = 2;
		theta  = theta * 180/CV_PI;
		bool draw = false;
		if( (theta < degThresh or 180 - degThresh < theta) and fits(x0, xs, gap)) {
			xs.insert(x0); draw = true;
		}

		if( abs(theta - 90) < degThresh and fits(y0, ys, gap)) {
			ys.insert(y0); draw = 1;
		}

			if (draw){
				Point pt1, pt2;
				pt1.x = cvRound(x0 + 1000*(-b));
				pt1.y = cvRound(y0 + 1000*(a));
				pt2.x = cvRound(x0 - 1000*(-b));
				pt2.y = cvRound(y0 - 1000*(a));
				line( imageL, pt1, pt2, 255, 1);
			}
	}
	imshow("imageC", imageC);
	imshow("imageL", imageL);
	vPos = vector<int>(xs.begin(), xs.end());
	hPos = vector<int>(ys.begin(), ys.end());
}

int main(int argc, char** argv )
{
	// Read image
	Mat orig = imread(argv[1], IMREAD_COLOR );

	Mat gray; cvtColor(orig, gray, CV_RGB2GRAY);

	// radius based thersholding
	Mat black = threshold(gray, 5);

	// blob extraction - https://en.wikipedia.org/wiki/Connected-component_labeling
	// graph theory
	map<int, int> stats;
	Mat blobs = blobExtract(black, stats);

	// find the blob label with the most elements
	//https://stackoverflow.com/questions/30611709/find-element-with-max-value-from-stdmap
	stats[0] = 0;
	auto maxlbl_count = std::max_element(stats.begin(), stats.end(), compare2nd<int,int>);
	int maxlbl = maxlbl_count -> first;
	cout << "maxlbl: " << maxlbl << endl;
	
	// get the corners of the original sudoku puzzle
	vector<Point2f> sdkCorners = getSudokuCorners(blobs, maxlbl);

	// create a square matrix to put the puzzle in
	Mat puzzle = Mat( 480, 480, CV_32FC1, Scalar(0));

	// get the corners of the puzzle image
	vector<Point2f> puzzleCorners = getCorners(puzzle.size()); 

	// transform the original sudoku puzzle to the new square matrix
	Mat HTranform = findHomography(sdkCorners, puzzleCorners,  RANSAC);
	warpPerspective(gray, puzzle, HTranform, puzzle.size());
	imshow("puzzle", puzzle);
	
	// get the positions of all the vertival and horizontal lines
	vector<int> vPos, hPos;
	getHVLinePos(puzzle, vPos, hPos);
	
	// calculate the number of cols and rows
	int sdkCols = vPos.size()-1;
	int sdkRows = hPos.size()-1;
	cout << "sdkCols: " << sdkCols << "\n";
	cout << "sdkRows: " << sdkRows << "\n";

	map<int, int> cellStats;
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	api->Init(NULL, "eng");
	api->SetVariable("tessedit_char_whitelist", "0123456789");

	float padding = 0.15;
	string outText;
	for(int j = 0; j<sdkRows; j++){
		for(int i = 0; i<sdkCols; i++){
			int index = i+sdkRows*j;
			int rWidth = vPos[i+1]-vPos[i];
			int rHeight = hPos[j+1]-hPos[j];
			int startX = vPos[i] + rWidth*padding;
			int startY = hPos[j] + rHeight*padding;

			Mat im = puzzle(Rect(startX, startY, rWidth*(1.0-padding), rHeight*(1.0-padding)));
			Mat thresh  = threshold(im,3);
			Mat blobs = blobExtract(thresh, cellStats);
			
			//cv::threshold(im,im, 200, 255, THRESH_TOZERO_INV);
			if(cellStats.size() > 1){
				printf( "%3i: ", index + 1 );
				// Open input image with leptonica library
				api->SetImage(thresh.data, thresh.cols, thresh.rows, 1, thresh.step);
				// Get OCR result
				outText = api->GetUTF8Text();
				
				cout << outText.c_str();
				
				if(outText.length() > 0){
					imshow(to_string(index+1),im);
					printf("\033[1;46;30m%3i \033[0m", stoi(outText));
				} else {
					cout << "    ";
				}
			}
		}
		cout <<"\n";
	}

	api->End();
	waitKey(0);
	exit(0);
}
