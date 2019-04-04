#include <iostream>
#include <set>
#include "digraph.cpp"

bool verbose = 0;

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
		void hLine(){
			cout << "  ";
			for(int i = 0; i < cols; i++)
				cout << "--------";
			cout << "\n";
		}
		void draw (){
			for(int j = 0; j < rows; j++){
				if(j % gheight == 0){
					hLine();
				}
				for(int i = 0; i < cols; i++){
					if(i % gwidth == 0)
						cout << " |";
					int k = graph.getCell(i,j)->k;
					cout << "\t";
					if(k==0) cout << "*";
					else cout << k;

					if(i+1 == cols)
						cout << " |";
				}
				cout << "\n";
			}
			hLine();
		}

		void makeRelations(){
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

		intVecCellMap getPsbltyMap(){
			int kolors = gwidth * gheight;
			vector<Cell*> cells = graph.getCellPtrs();
			intVecCellMap psbltyMap;
			for(auto A: cells){
				if (A->k > 0) continue;
				int cnvrgCount = A->relations.size();
				int psbltyCount = kolors - cnvrgCount;
				psbltyMap[psbltyCount].push_back(A);
			}
			return psbltyMap;
		}

		void solveOnePsblty(intVecCellMap psbltyMap){
			// get the cell
			for (auto pCell:psbltyMap[1]){
				if(verbose) cout << "one solution: " << *pCell << "\n";
				set<int> connectedKs;
				for(auto r:pCell->relations){
					connectedKs.insert(r.k);
				}
				int newK;
				for(newK = 1; connectedKs.count(newK); newK++);

				if(verbose) cout << "solution: " << newK << "\n";
				pCell->k = newK;
			}

		}
		
		void solveLeastPsblty(intVecCellMap psbltyMap, int leastKey){
			vector<Cell*> lPsbltyCells = psbltyMap[leastKey];
			// get the best candidate
			int maxCount = -1; Cell* lPsbltyCell;
			for(auto lpCell : lPsbltyCells){
				int filledCount = 0;
				for(auto lpcr : getRelatives(lpCell)){
					if(lpcr->k > 0) filledCount++;
				}
				if(verbose) cout << *lpCell 
					<< "--(filled neighbours)-->"
						<< filledCount << "\n";
				if(filledCount > maxCount){
					lPsbltyCell = lpCell;
					maxCount = filledCount;
				}
			}

			if(verbose) cout << leastKey 
				<< " solutions: " 
					<< *lPsbltyCell << "\n";
			set<int> connectedKs;
			for(auto r:lPsbltyCell->relations){
				connectedKs.insert(r.k);
			}
			int newK;
			for(newK = 1; connectedKs.count(newK); newK++);
			if(verbose) cout << "solution: " << newK << "\n";
					lPsbltyCell->k = newK;

		}

		void solve(){
			bool solved = false;
			while(!solved){
				// make all neccesary relations
				makeRelations();
				// get the psbltyMap
				intVecCellMap psbltyMap = getPsbltyMap();
				if(verbose){
					cout << "----------\nPossibilty Map\n----------\n";
					for(auto p:psbltyMap){
						cout << p.first << ": ";
						for(auto v:p.second){
							cout << *v << "; ";
						}
						cout << "\n";
					}
					cout << "----------\n";
				}
				if(psbltyMap.size() == 0){
					solved = true; break;
				} else if ( psbltyMap.count(0) ){
					break;
				}

				// check if there are Cells with only 1 possible solution
				int leastKey = psbltyMap.begin()->first;
				if(verbose) cout << "least posibilities: " << leastKey << "\n";
				if (leastKey == 1){
					solveOnePsblty(psbltyMap);
				} else {
					solveLeastPsblty(psbltyMap, leastKey);
				} 
			}
			if(!solved){
				cout << "PUnsolvable puzzle\n";
			}
		}

};

