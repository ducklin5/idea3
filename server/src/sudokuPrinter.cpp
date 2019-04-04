// ---------------------------------------------------
//    Name: Adekunle Toluwani & Azeez  Abass
//    ID's: 1538562 & 1542780
//    CMPUT 275 EB1, Winter 2019
// ---------------------------------------------------

#include <iostream> // cout for debugging
#include <vector> // vector class
#include <string>  // string class
#include <stack>   // stack class
#include "serialport.h" // Serial Communication
#include "Sudoku.cpp"

using namespace std;

void wait4Prompt( SerialPort &inSerial){
	// *********************
	// Wait for arduino cli to be available
	// *********************
	string inputline; // where line will be stored
	
	cout << "Waiting for prompt \n";
	// wait for prompt to be available from the arduino
	do {
		// get a line and check for eof
		inputline = inSerial.readline();
		cout << inputline;
	} while (inputline != "R >\n");
}

void drawLine(SerialPort &ios, float x0,  float y0, float x1, float y1 ){
	ios.writeline("G01 X");
	ios.writeline(to_string(x0));
	ios.writeline(" Y");
	ios.writeline(to_string(y0));
	ios.writeline("\n");
	wait4Prompt(ios);

	ios.writeline("G00 Z30\n");
	wait4Prompt(ios);

	ios.writeline("G01 X");
	ios.writeline(to_string(x1));
	ios.writeline(" Y");
	ios.writeline(to_string(y1));
	ios.writeline("\n");
	wait4Prompt(ios);

	ios.writeline("G00 Z00\n");
	wait4Prompt(ios);
}

void printPuzzle(SerialPort &ioSerial, Sudoku puzzle){
	wait4Prompt(ioSerial);
	drawLine(ioSerial,0,0,0,100);
	drawLine(ioSerial,0,100,100,100);
	drawLine(ioSerial,100,100,100,0);
	drawLine(ioSerial,100,0,0,0);
	
}

int main(int argc, char* argv[]){
	
	cout << "Please enter width of sudoku puzzle: ";
	int width;
	cin >> width;
	cout << "Please enter height of the sudoku puzzle: ";
	int height;
	cin >> height;

	cout << "Please enter group width of sudoku puzzle: ";
	int gwidth;
	cin >> gwidth;

	cout << "Please enter group height of the sudoku puzzle: ";
	int gheight;
	cin >> gheight;

	//SerialPort ioSerial("/dev/ttyACM0");
	
	Sudoku puzzleA(width,height,gwidth,gheight);
	//printPuzzle(ioSerial, puzzleA);

	puzzleA.draw();

	cout << "Please enter the number of the hints: ";
	int hintNum;
	cin >> hintNum;

	set<int> hintIndx;
	int index;

	if(hintNum > 0){
		cout << "Please enter the space seperated postions (by index 1) of the hints: ";
	}

	for(int i = 0; i < hintNum; i++){
		cin >> index;
		hintIndx.insert(index-1);
	}

	vector<int> hints;
	int val;
	for(int i = 0; i < width * height; i++){
		if (hintIndx.count(i) == 0) {hints.push_back(0); continue;};
		do{ 
			cout << "Enter value at position " << i + 1 << ": " ; 
			cin >> val;
			if(val > gwidth*gheight){
				cout << val << " does not fit in this puzzle. Please try again: \n";
			}
		} while (val>gwidth*gheight);
		hints.push_back(val);
	}

	puzzleA.populate(hints);
	if(hintNum > 0){
		puzzleA.draw();
	}
	puzzleA.solve();
	puzzleA.draw();
	
}

