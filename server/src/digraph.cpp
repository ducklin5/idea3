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
	Relation( Cell& fromC, Cell& toC, int kvar = 0 ) {
		from = &fromC;
		to = &toC;
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
	void addRelation(Cell &to, int kVar=0){
		Relation newRelation(*this, to, kVar);
		relations.push_back(newRelation);
	}
	bool operator==(const Cell& b) const{ 
		return (b.i == i && b.j == j);
	}
	friend std::ostream& operator<<(std::ostream&, const Cell&);

};

namespace std
{
	template <>
		struct hash<Cell>
		{
			size_t operator()(const Cell& b) const
			{
				return ((hash<int>()(b.i) ^ (hash<int>()(b.j) << 1)) >> 1);
			}
		};
		
}
ostream& operator<< (ostream& os, const Cell& sElem) {
	os << sElem.i << ", " << sElem.j << " [" << sElem.k << "]";
	return os;
}

class CellGraph{
	public:
		void addCell(Cell &inCell){
			// inserts a new cell into the graph
			Cell* cellPtr = &inCell;
			cells.insert(cellPtr);
		}
		void addRelation(Cell &cellA, Cell &cellB){
			addCell(cellA); addCell(cellB);
			
			cellA.addRelation(cellA);
			cellB.addRelation(cellB);
		}

		unordered_set<Cell*> getCells() {
			return cells;
		}

		void print(){
			for(auto cell: cells){
				cout << *cell;
				cout << " --> ";
				for(auto relation:cell->relations){
					cout << *relation.to;
					cout << "; ";
				}
				cout << "\n";
			}
		}
	private:
		unordered_set<Cell*> cells;
};

//int main(){
//	CellGraph sdk;
//	Cell someCell(0,0,0);
//	sdk.addCell(someCell);
//	Cell someCell2(1,0,0);
//	sdk.addCell(someCell2);
//	sdk.addRelation(someCell, someCell2);
//
//	sdk.print();
//
//
//}
