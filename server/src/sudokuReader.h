#ifndef SDK_RDR_H
#define SDK_RDR_H

#include <vector>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <algorithm>
#include <set>
#include <map>
using namespace std;
/* reads a cropped sudoku image, with vertical line in vPos
 * and horizontal lines in hPos
 * Input: cropped/focused/oriented puzzle image Matrix
 * - vPos (vertical postitons) and hPos (horizontal positions)
 * Output:
 * - vector of integers in teh sudoku puzzle (0 indexed); */
vector<int> readSudokuImage(char* filename, int& width, int& height);

#endif
