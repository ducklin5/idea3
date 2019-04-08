#include "sudokuReader.h"
#define THRESH_CONSTANT 0.9

using namespace std;
using namespace cv;
/**************************
 * HELPER FUNCTIONS
 **************************/
// compare the second item in a pair
template <typename fType, typename sType>
bool compare2nd (const pair<fType, sType>& p1, const pair<fType, sType>& p2){
	return p1.second < p2.second;
}
// given an int set where all the numbers must be `space` way from each other
// check if `num` can fit in that set
bool fits (int num, set<int> vec, int space ){
	bool pass = true;
	for( auto elem : vec )
		if (abs (num - elem) < space ) { pass = false; break; }
	return pass;
}

// given a rectangle Size object
// return a vector of its corner points
vector<Point2f> getCorners(Size pSize){
	vector<Point2f> corners = {
		Point2f(0,0),
		Point2f(pSize.width, 0),
		Point2f(pSize.height, pSize.width),
		Point2f(0, pSize.height)
	};
	return corners;
}
/***************************/

/* Function that applies and adaptive threshold to an image
 * This means the image is made black and white for each pixel by using
 * the neighbouring pixels 
 * Input: a grayscale image matrix
 * Output: a black and white image matrix */
Mat threshold (Mat img, int radius) {
	Mat outImg(img.size(), CV_8UC1);
	// for each pixel
	for (int y = 0; y < img.rows; y++){ for (int x = 0; x < img.cols; x++){
		float avg = 0;
		int count = 0;
		// loop through surrounding pixels
		for(int j = -radius; j <= radius; j++){
			// next if the neighbor doesnt exist (y)
			if(y+j < 0 or y+j >= img.rows) continue; 
			for(int i = -radius; i <= radius; i++){
				// next if the neighbor doesnt exist (x)
				if(x+i < 0 or x+i >= img.rows) continue;
				// add to the average
				avg += img.at<uchar>(y+j,x+i); count ++;
			}
		}
		// caluclate the average value of all its neigbours
		avg /= count;
		outImg.at<uchar>(y,x) = (img.at<uchar>(y, x) < avg * THRESH_CONSTANT) * 255;
	}}
	return outImg;
}

/* Given a black and white image this function will identify different items in the image.
 * Input: black and white image matrix
 * Output: matrix of values indicating which label/item the pixel belongs to */
// https://www.youtube.com/watch?v=hMIrQdX4BkE
Mat blobExtract( Mat binImg, map<int,int>& stats){
	Mat blobImg(binImg.size(), CV_32SC1, Scalar(0));
	int label = 0;

	// contains lists of all the equivalent labels
	map <int,set<int>> labelEqvs;

	// first pass -- loop through all pixels
	for (int y = 0; y < binImg.rows; y++){ for (int x = 0; x < binImg.cols; x++){
		uchar& currentPx = binImg.at<uchar>(y,x);
		int& currentLabel = blobImg.at<int>(y,x);
		
		// only procced for white pixels
		if(currentPx <= 0) continue;
		
		// intialize variable to store top and left data (-1 means never set)
		int top = -1; int left = -1;
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
			int toplbl = 0; int leftlbl = 0;
			if ( top > 0 ) {
				toplbl = blobImg.at<int>(y-1, x);
				currentLabel = toplbl;
			}
			if ( left > 0 ){
				leftlbl = blobImg.at<int>(y, x-1);
				currentLabel = blobImg.at<int>(y, x-1);
			}
			// if they both are foreground and they have differnt labels
			if ( top > 0 and left > 0 and toplbl != leftlbl ){
				// set the label to the lowest one and record the conflict
				currentLabel = ( toplbl < leftlbl ? toplbl: leftlbl);
				labelEqvs[toplbl].insert(leftlbl);
				labelEqvs[leftlbl].insert(toplbl);
			}
		}
	}}
	
	// simplify the equivalence map
	map <int,int> simpleEqvs;
	for (auto const& eqv: labelEqvs){
		// for each label find it smallest equivalence by tracing back untill there is no smaller one
		int crnt = eqv.first;
		while (labelEqvs[crnt].size() > 0){
			if(*labelEqvs[crnt].begin() < crnt){
				crnt = *labelEqvs[crnt].begin();
			} else {
				break;
			}
		}
		// finally the smallest equivalence for the label is added
		simpleEqvs[eqv.first] = crnt;
	}

	// second pass
	for (int y = 0; y < blobImg.rows; y++){ for (int x = 0; x < blobImg.cols; x++){
		// for each pixel re-label it with the appropriate smallest label
		int& currentLabel = blobImg.at<int>(y,x);
		// get the first element of the set/ the lowest equivalent
		currentLabel = simpleEqvs[currentLabel];
		// update the number of pixes in this label
		if (stats.count(currentLabel)==0) {
			stats[currentLabel] = 1;
		} else {
			stats[currentLabel]++;
		}
	}}

	return blobImg;
}


/* given a blob matrix and a label interger (that should exist in the matrix)
 * it will return the a vector of the pixles in the blob closest to each corner
 * Input: blob matrix and label int
 * Output: vector of corner points */
vector<Point2f> getSudokuCorners(Mat blobs, int puzzlelbl){
	// isolate/mask the blob of label `puzzlelbl`
	// and calculate the distances from each img corner to each mask pixel
	
	// get the corners of the original image
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
	
	//for (int i=0; i<4; i++) imshow(to_string(i), dists[i] * 50);

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

/* Detects the position of horizontal and verticals lines in a sudoku image
 * Input: teh sudoku image
 * Outputs: populates the vPos and hPos vectors with the approriate values */
void getHVLinePos(Mat image, vector<int>& vPos, vector<int>& hPos){
	float gap = 0.05*image.cols; // line must be atleast 5% of the image width apart
	int degThresh = 2; // lines must be ateast 2 degrees from horizontal/vertical
	// Canny edge detection to detect all edges
	Mat imageC;
	cv::Canny(image,imageC,30,20);
	// vector to hold all the line data
	vector<Vec2f> lines;
	// find all line from the canny image
	HoughLines(imageC, lines, 1, CV_PI/180, 80, 0, 0 );
	
	set<int> xs = {0, image.cols-1};
	set<int> ys = {0, image.rows-1};
	
	// for each line
	image *= 0.4;
	cvtColor(image, image, CV_GRAY2BGR); 
	for( size_t i = 0; i < lines.size(); i++ ){
		// calculate its center position
		bool draw = 0;
		float rho = lines[i][0], theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		int x0 = a*rho, y0 = b*rho;

		theta  = theta * 180/CV_PI;
		// if it is veritical and it fits in the set
		if( (theta < degThresh or 180 - degThresh < theta) and fits(x0, xs, gap)) {
			xs.insert(x0); draw=1; // add it
			// if it is horizontal and it fits in the set
		} else if( abs(theta - 90) < degThresh and fits(y0, ys, gap)) {
			ys.insert(y0); draw=1;//add it
		}
		if (draw){
			Point pt1, pt2;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			line( image, pt1, pt2, Scalar(0,255,0), 1);
		}
	}
	imshow("Lines", image);
	vPos = vector<int>(xs.begin(), xs.end());
	hPos = vector<int>(ys.begin(), ys.end());
}

vector<int> getSudokuNumbers(Mat puzzle, vector<int> vPos, vector<int> hPos){
	// calculate the number of cols and rows
	int sdkCols = vPos.size()-1;
	int sdkRows = hPos.size()-1;
	cout << "sdkCols: " << sdkCols << "\n";
	cout << "sdkRows: " << sdkRows << "\n";
	// get image ready for ocr
	// very tricky step
	Mat gaussBlur;
	cv::GaussianBlur(puzzle, gaussBlur, Size(0,0), 1);
	cv::addWeighted(puzzle, 1.7, gaussBlur, -0.7, 0, puzzle);
	puzzle = threshold(puzzle, puzzle.cols*0.008);

	imshow("puzzle4Ocr", puzzle);
	// start up the ocr engine
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	api->Init(NULL, "eng");
	// only allow numbers
	api->SetVariable("tessedit_char_whitelist", "0123456789");
	
	float padding = 0.1; // fractional padding of each cell
	vector<int> values (sdkCols * sdkRows);
	// for each cell
	for(int j = 0; j<sdkRows; j++){ for(int i = 0; i<sdkCols; i++){
		int index = i+sdkRows*j;
		printf( "%3i: ", index + 1 );
		
		// calculate the starting position and size of each cell
		int rWidth = vPos[i+1]-vPos[i];
		int rHeight = hPos[j+1]-hPos[j];
		int startX = vPos[i] + rWidth*padding;
		int startY = hPos[j] + rHeight*padding;
		rWidth *= (1.0-padding*2);
		rHeight *= (1.0-padding*2);

		// get the sub image (cell image)
		Mat im = puzzle(Rect(startX, startY, rWidth, rHeight));
		// find all the blobs in the image
		map<int,int> cellStats;
		Mat blobs = blobExtract(im, cellStats);
		
		bool NaN = false;
		// if there is a blob then there is a number.. read it
		if(cellStats.size() >= 1){
			api->SetImage(im.data, im.cols, im.rows, 1, im.step);
			// Get OCR result
			string outText = api->GetUTF8Text();
			
			int num = atoi(outText.c_str());
			if(outText.length() > 0 and num > 0 and num <= sdkCols * sdkRows){
				printf("\033[1;46;30m%3i \033[0m",num);
				values[index] = num;
				//imshow(to_string(index+1), im);
			} else { NaN = 1; }
		} else { NaN = 1; }
		
		if(NaN){
			values[index] = 0;
			cout << "    ";
		}
	} cout <<"\n"; }
	api->End();
	return values;
} 

/* reads a cropped sudoku image, with vertical line in vPos
 * and horizontal lines in hPos
 * Input: cropped/focused/oriented puzzle image Matrix
 * - vPos (vertical postitons) and hPos (horizontal positions)
 * Output:
 * - vector of integers in teh sudoku puzzle (0 indexed); */
vector<int> readSudokuImage(char* filename, int& width, int& height ) {
	// Read image
	Mat orig = imread(filename, IMREAD_COLOR);
	Mat gray; cvtColor(orig, gray, CV_RGB2GRAY);

	// radius based thersholding
	Mat black = threshold(gray, 6);
	// filled it up a bit to make sure components aren't broken
    dilate(black, black, Mat());

	// connected component detection
	map<int, int> stats;
	Mat blobs = blobExtract(black, stats);

	// find the blob label / item with the most pixels
	//https://stackoverflow.com/questions/30611709/find-element-with-max-value-from-stdmap
	stats[0] = 0; // dont care about the background label
	auto maxlbl_count = std::max_element(stats.begin(), stats.end(), compare2nd<int,int>);
	int maxlbl = maxlbl_count -> first;
	
	// get the corners of the item / original sudoku puzzle
	vector<Point2f> sdkCorners = getSudokuCorners(blobs, maxlbl);
	
	// find the longest edge
	float maxLength = 0;
	for(int i=0; i<4; i++){
		float len = cv::norm(sdkCorners[i]-sdkCorners[(i+1)%4]);
		if(len > maxLength) maxLength = len;
	}
	
	// create a square matrix to put the puzzle in
	Mat puzzle = Mat( maxLength, maxLength, CV_32FC1, Scalar(0));

	// get the corners of the puzzle image
	vector<Point2f> puzzleCorners = getCorners(puzzle.size());

	// transform the original sudoku puzzle to the new square matrix
	Mat HTranform = findHomography(sdkCorners, puzzleCorners,  RANSAC);
	warpPerspective(gray, puzzle, HTranform, puzzle.size());
	imshow("puzzle", puzzle);
	
	// threshold it by 2% of the width
	Mat puzzleA = threshold(puzzle, maxLength*0.02);
    dilate(puzzleA, puzzleA, Mat());
	
	// get the positions of all the vertical and horizontal lines
	vector<int> vPos, hPos;
	getHVLinePos(puzzleA, vPos, hPos);
	
	// get the width and heght of the puzzle
	width = vPos.size()-1; height = hPos.size()-1;

	// read all the values in the puzzle
	vector<int> values = getSudokuNumbers(puzzle, vPos, hPos);
	
	waitKey(0);
	return values;
}
