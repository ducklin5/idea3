#include <map> // primary cell data strcute
#include <vector> // several uses
#include <utility> // pair
#include <iostream> // for printing cell structures
#include <cassert> // assert, error check

using namespace std;

// A vertex structure
struct Cell;

// An edge structure
struct Relation{
	Cell* from;
	Cell* to;
	int k;
	Relation( Cell* fromC, Cell* toC, int kvar = 0 ) {
		from = fromC;
		to = toC;
		k = kvar;
	}
};

struct Cell{
	// position properties
	int x; 
	int y;
	int n;
	// value property
	int k;
	vector<Relation> relations;
	Cell(int xvar, int yvar, int nvar = 0, int kvar = 0) {
		x = xvar;
		y = yvar;
		k = kvar;
		n = nvar;
	}
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

// graph class using Cell vertex and Relation Edges
class CellGraph{
	public:
		Cell* addCell(int x, int y, int n = 0){
			// check if a pointer to that cell already exists
			if (cells.count({x,y}) > 0)
				return cells[{x,y}];
			// inserts a new cell into the graph
			Cell* newCellPtr = new Cell(x,y,n);
			cells[{x,y}] = newCellPtr;
			return newCellPtr;
		}
		Cell* getCell(int x, int y){
			bool exists = cells.count({x,y}) > 0;
			if(!exists){
				cout << x << ", " << y;
			}
			assert (exists && "No such cell in graph");
			return cells[make_pair(x,y)];
		}
		void addRelation(Cell* cellA, Cell* cellB, int k){
			Cell* A = addCell(cellA->x, cellA->y);
			Cell* B = addCell(cellB->x, cellB->y);
			
			A->addRelation(B, k);
			B->addRelation(A, k);
		}

		vector<Cell*> getCellPtrs() {
			vector<Cell*> cellPtrVec;
			 for(auto elem : cells)
				 	 cellPtrVec.push_back(elem.second);
			return cellPtrVec;
		}

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
