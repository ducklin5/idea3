#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <iostream>

using namespace std;

struct Cell;
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
	int i;
	int j;
	int n;
	int k;
	vector<Relation> relations;
	Cell(int ivar, int jvar, int nvar = 0, int kvar = 0) {
		i = ivar;
		j = jvar;
		k = kvar;
		n = nvar;
	}
	void addRelation(Cell* to, int kVar=0){
		Relation newRelation(this, to, kVar);
		relations.push_back(newRelation);
	}
	bool operator==(const Cell& b) const{ 
		return (b.i == i && b.j == j);
	}
	friend std::ostream& operator<<(std::ostream&, const Cell&);

};

ostream& operator<< (ostream& os, const Cell& sElem) {
	os <<"<"<< sElem.i << ", " << sElem.j << ", " << sElem.n << ">[" << sElem.k << "]";
	return os;
}

class CellGraph{
	public:
		Cell* addCell(int i, int j){
			// create a new cell
			Cell* newCellPtr = new Cell(i,j);
			// check if a pointer to that cell already exists
			for(auto cellPtr : cells)
				if (*cellPtr == *newCellPtr) return cellPtr;
			// inserts a new cell into the graph
			cells.push_back(newCellPtr);
			return newCellPtr;
		}
		void addRelation(Cell* cellA, Cell* cellB){
			
			Cell* A = addCell(cellA->i, cellA->j);
			Cell* B = addCell(cellB->i, cellB->j);
			
			A->addRelation(B);
			B->addRelation(A);
		}

		vector<Cell*> getCellPtrs() {
			return cells;
		}

		void print(){
			for(auto cell: cells){
				cout << *cell;
				cout << " --> ";
				for(auto r:cell->relations){
					cout << *r.to;
					cout << "; ";
				}
				cout << "\n";
			}
		}
	private:
		vector<Cell*> cells;
};


