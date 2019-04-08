// ---------------------------------------------------
//    Name: Adekunle Toluwani & Azeez  Abass
//    ID's: 1538562 & 1542780
//    CMPUT 275 EB1, Winter 2019
// ---------------------------------------------------

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream> // cout for debugging
#include <vector> // vector class
#include <string>  // string class
#include <stack>   // stack class
#include "sudokuReader.h" // for reading a sudoku image file
#include "Sudoku.cpp"

using namespace std;

int main(int argc, char* argv[]){
	int width, height;
	vector<int> hints;
	
	if(argc==1){
		cout << "Please enter width of sudoku puzzle: ";
		cin >> width;
		cout << "Please enter height of the sudoku puzzle: ";
		cin >> height;
		hints.resize(width*height, 0);
	} else {
		hints = readSudokuImage(argv[1], width, height);
	}

	cout << "Please enter group width of sudoku puzzle: ";
	int gwidth;
	cin >> gwidth;

	cout << "Please enter group height of the sudoku puzzle: ";
	int gheight;
	cin >> gheight;
	
	Sudoku puzzleA(width,height,gwidth,gheight);
	puzzleA.populate(hints);
	puzzleA.draw();
	
	char update;
	do {
		cout << "Would you like to update some values? (y/n) ";
		cin >> update;
	} while ( update != 'y' and update != 'n');

	if(update == 'y'){
		cout << "enter 0 when completed\n";
		int index; int val;
		while (true){
			cout << "Position: "; cin >> index;
			if( index <= 0 or index>width*height) break;
			index -= 1;
			cout << "Value: "; cin >> val;
			if( val == 0) break;
			if(val > gwidth*gheight){
				cout << val << " does not fit in this puzzle. Please try again.";
				continue;
			}
			hints[index] = val;
			puzzleA.populate(hints);
			puzzleA.draw();
		}
	}

	puzzleA.draw();
	puzzleA.solve();
	puzzleA.draw();
	
}

