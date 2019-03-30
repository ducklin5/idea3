#include <iostream>
#include "digraph.cpp"

class Sudoku {
	public:
		CellGraph graph;
		int rows;
		int cols;

		Sudoku(int r = 3, int c = 3, int gw = 3, int gh = 3 ){
			rows = r;
			cols = c;
			for(int y = 0; y < rows; y++){
				for(int x = 0; x < cols; x++){
					int n = x/gw;
					graph.addCell(x,y,n);
				}
			}
		}
		
		void addData(int x, int y, int k){
			Cell* B = graph.getCell(x,y);
			B->k = k;
		}

};

int main(){
	Sudoku puzzleA(4,4,2,2);
	puzzleA.addData(0,0,1);
	puzzleA.addData(0,2,2);
	puzzleA.addData(1,1,4);
	puzzleA.addData(2,2,3);
	puzzleA.addData(3,3,2);

	puzzleA.graph.print();

}
