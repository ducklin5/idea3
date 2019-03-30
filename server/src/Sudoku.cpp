#include <iostream>
#include <set>
#include "digraph.cpp"

typedef map<int,vector<Cell*>> intVecCellMap;
class Sudoku {
	private:
		CellGraph graph;
		int rows;
		int cols;
		int gwidth;
		int gheight;
		map<int,vector<Cell*>> groups;

	public:
		Sudoku(int r = 3, int c = 3, int gw = 3, int gh = 3 ){
			rows = r; gwidth = gw;
			cols = c; gheight = gh;
			for(int y = 0; y < rows; y++){
				for(int x = 0; x < cols; x++){
					int n = calcGroup(x,y);
					Cell* A = graph.addCell(x,y,n);

					groups[n].push_back(A);
				}
			}
		}

		int calcGroup(int x, int y){
			int colGroups = cols/gwidth;
			return x/gwidth + colGroups * (y/gheight);
		}

		void setVal(int x, int y, int k){
			Cell* B = graph.getCell(x,y);
			B->k = k;
		}
		
		void populate(vector<int> input){
			for(int y = 0; y < rows; y++){
				for(int x = 0; x < cols; x++){
					setVal(x,y,input[x+y*cols]);
				}
			}
		}

		set<Cell*> getRelatives(Cell* input){
			int x = input -> x;
			int y = input -> y;
			set<Cell*> relatives;
			for(int i = 0; i < cols; i++ ){
				relatives.insert(graph.getCell(i,y));
			}
			for(int j = 0; j < rows; j++){
				relatives.insert(graph.getCell(x,j));
			}
			int n = calcGroup(x,y);
			for ( auto groupCell : groups[n] ){
				relatives.insert(groupCell);
			}
			relatives.erase(input);
			return relatives;
		}

		void draw (){
			for(int j = 0; j < rows; j++){
				if(j % gheight == 0){
					for(int j = 0; j < rows; j++)
						cout << "---------";
					cout << "\n";
				}
				for(int i = 0; i < rows; i++){
					if(i % gwidth == 0)
						cout << " |";
					cout << "\t" << graph.getCell(i,j)->k;
				}
				cout << "\n";
			}
			for(int j = 0; j < rows; j++)
				cout << "---------";
			cout << "\n";
		}

		void solveStep1(){
			vector<Cell*> cells = graph.getCellPtrs();
			for(auto A: cells){
				if (A->k == 0) continue;
				for(auto B : getRelatives(A)){
					if (B->k > 0) continue;
					bool alreadyConnected = false;

					for(auto r : B->relations){
						alreadyConnected = (r.k == A->k);
						if (alreadyConnected) break;
					}

					if(alreadyConnected) continue;
					graph.addRelation(A,B, A->k);
				}
			}
		}

		intVecCellMap solveStep2(){
			int kolors = gwidth * gheight;
			vector<Cell*> cells = graph.getCellPtrs();
			intVecCellMap psbltyMap;
			for(auto A: cells){
				if (A->k > 0) continue;
				int cnvrgCount = A->relations.size();
				int psbltyCount = kolors - cnvrgCount;
				psbltyMap[psbltyCount].push_back(A);
			}
//			for(auto elem:psbltyMap){
//				cout << elem.first << " #:# ";
//				for(auto cell:elem.second){
//					cout << *cell << "; ";
//				}
//				cout << "\n";
//			}
			return psbltyMap;
		}

		void solveStep3(intVecCellMap psbltyMap){
			// get the cell
			for (auto pCell : psbltyMap[1]){
				cout << "one solution: " << *pCell << "\n";
				set<int> connectedKs;
				for(auto r:pCell->relations){
					connectedKs.insert(r.k);
				}
				int newK;
				for(newK = 1; connectedKs.count(newK); newK++);
				cout << "solution: " << newK << "\n";
				pCell->k = newK;
			}
		}

		void solve(){
			bool solved = false;
			while(!solved){
				this->draw();
				// make all neccesary relations
				solveStep1();
				// get the psbltyMap
				intVecCellMap psbltyMap = solveStep2();
				if(psbltyMap.size() == 0){
					solved = true; break;
				} else if ( psbltyMap.count(0) ){
					break;
				}

				// check if there are Cells with only 1 possible solution
				int leastKey = psbltyMap.begin()->first;
				cout << "least posibilities: " << leastKey << "\n";
				cout << "candidates: ";
				for( auto cell : psbltyMap[leastKey] ){
					cout << *cell << "; ";
				}
				cout << "\n";
				if (leastKey == 1){
					solveStep3(psbltyMap);
				} else {
					vector<Cell*> lPsbltyCells = psbltyMap[leastKey];
					// get the best candidate
					int maxCount = 0; Cell* lPsbltyCell;
					for(auto lpCell : lPsbltyCells){
						int count = 0;
						for(auto lpcr : getRelatives(lpCell))
							if(lpcr->k == 0) count++;
						if(count > maxCount){
							lPsbltyCell = lpCell;
							maxCount = count;
						}
					}

					cout << leastKey << " solutions: " << *lPsbltyCell << "\n";
					set<int> connectedKs;
					for(auto r:lPsbltyCell->relations){
						connectedKs.insert(r.k);
					}
					int newK;
					for(newK = 1; connectedKs.count(newK); newK++);
					cout << "solution: " << newK << "\n";
					lPsbltyCell->k = newK;
				} 
			}
			if(!solved){
				cout << "What the heck is this puzzle fam!!!???\n";
			}
		}

};

int main(){
	Sudoku puzzleA(6,6,3,2);
	vector<int> hints{
		0, 0, 4, 3, 0, 0,
		0, 3, 6, 0, 0, 0,
		0, 0, 0, 5, 2, 4,
		2, 4, 5, 0, 0, 0,
		0, 0, 0, 4, 5, 0,
		0, 0, 1, 6, 0, 0
	};
	puzzleA.populate(hints);
	puzzleA.solve();
}
