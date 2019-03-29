#include <iostream>
#include "digraph.cpp"

class Sudoku {
	public:
		CellGraph graph;
		int rows;
		int cols;

		Sudoku(int r = 3, int c = 3 ){
			rows = r;
			cols = c;
			for(int i = 0; i < cols; i++){
				for(int j = 0; j < rows; j++){
					unordered_set<Cell*> existing = graph.getCells();
					Cell cellA(i,j, 3*(j/3) + (i/3));
					graph.addCell(cellA);
					for ( auto cellPtr : existing){
						if (cellA.i == cellPtr->i or cellA.j == cellPtr->j){
							graph.addRelation(cellA, *cellPtr);
						}
					}
				}
			}

		}
		
		void addData(int i, int j, int k){
		}

};

int main(){
	Sudoku puzzleA;
	puzzleA.graph.print();
	puzzleA.addData(0,0,20);

}
