#include <map> // primary cell data strcute
#include <vector> // several uses
#include <utility> // pair
#include <iostream> // for printing cell structures
#include <cassert> // assert, error check

// This file contains the description and definition of the Cell struct used in the main sudoku puzzle and the puzzle solver


using namespace std;

// A vertex structure that represents a cell in the sudoku puzzle
struct Cell;

// An edge structure that connects two cells. This particular relation is bidirectional i.e. if cell A has a relation with cell B,
// then cell B has that same relation with cell A. Relations are simply any cell that is located in the same row or column or group
// as another
 
struct Relation{
	Cell* from;
	Cell* to;
	int k;
	Relation( Cell* fromC, Cell* toC, int kvar = 0 ) {
		from = fromC;
		to = toC;
		k = kvar; // weighting of the edge. 
	}
};

struct Cell{
	// position properties
	int x; 
	int y;
	int n;
	// value property
	int k;
	vector<Relation> relations; // vector storing all the relations it has
	Cell(int xvar, int yvar, int nvar = 0, int kvar = 0) {	// cell constructor
		x = xvar;
		y = yvar;
		k = kvar;
		n = nvar;
	}
	
	// Method to add a relation to the vector of relations possesed by a cell
	
	void addRelation(Cell* to, int kVar=0){
		Relation newRelation(this, to, kVar);
		relations.push_back(newRelation);
	}
	// allow override << operator for cout
	friend std::ostream& operator<<(std::ostream&, const Cell&);
};

// for printing a Cell object
ostream& operator<< (ostream& os, const Cell& sElem) {
	os <<"<"<< sElem.x << ", " << sElem.y << ", " << sElem.n << ">[" << sElem.k << "]";
	return os;
}

// graph class using the Cell vertex struct and and Relation Edges struct
class CellGraph{
	public:
	
		// Method to add cell to the private pairwise integer and cell pointer map
		Cell* addCell(int x, int y, int n = 0){
			// check if a pointer to that cell already exists and return the cell pointer if it already does
			if (cells.count({x,y}) > 0)
				return cells[{x,y}];
			// inserts a new cell into the graph with the coordinates speciffied
			Cell* newCellPtr = new Cell(x,y,n);
			// add the newly created cell to the pairwise integer and cell pointer map
			cells[{x,y}] = newCellPtr;
			return newCellPtr;
		}

		// Method to look for and return a cell from the private map
		Cell* getCell(int x, int y){
			bool exists = cells.count({x,y}) > 0; // check if there is an occurence of the cell in the map and return true if there is
			if(!exists){
				cout << x << ", " << y;
			}
			assert (exists && "No such cell in graph"); // print out an error message if no such cell exists
			return cells[make_pair(x,y)]; // return the x and y coordinates of the cell if it exists
		}
		
		// Method to add a bidirectional relation to a cell by first of all adding the relation from A to B and then from B to A
		// ensuring the relation edge is bidirectional
		void addRelation(Cell* cellA, Cell* cellB, int k){
			Cell* A = addCell(cellA->x, cellA->y);
			Cell* B = addCell(cellB->x, cellB->y);
			
			A->addRelation(B, k);
			B->addRelation(A, k);
		}

		// Method to get a list of all the cell pointers in the map. Returns a vector of cell pointers
		vector<Cell*> getCellPtrs() {
			vector<Cell*> cellPtrVec;
			 for(auto elem : cells)
				 	 cellPtrVec.push_back(elem.second);
			return cellPtrVec;
		}
		
		// Printing the details of a cell
		void print(){
			for(auto elem: cells){
				cout << *elem.second;
				cout << " --> ";
				for(auto r:elem.second->relations){
					cout << *r.to;
					cout << "; ";
				}
				cout << "\n";
			}
		}
	
	// Key value Map that stores the position of a cell as its key and the cell pointer as its value
	private:
		map<pair<int,int>, Cell*> cells;
};

//int main(){
//
//	CellGraph cg;
//	for(int i= 0; i<10; i++)
//		for(int j=0; j<10; j++)
//			cg.addCell(i,j);
//
//	for(int i = 0; i < 10; i++){
//		if(i == 3 ) continue;
//		Cell* A = cg.getCell(i, 0);
//		cout << *A;
//	}
//
//}
