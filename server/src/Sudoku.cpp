#include <iostream>
#include <set>
#include "digraph.h"

// This file contains the sudoku puzzle class itself which utilizes the cell struct, the relations struct and the cellgraph class
// in addition to the methods that will be used to create, populate and solve the puzzle

bool verbose = 0;

// Key value map used later when solving the puzzle
typedef map<int,vector<Cell*>> intVecCellMap;

class Sudoku {
	private:
		CellGraph graph;
		// size and physical properties
		int rows;
		int cols;
		int gwidth;
		int gheight;
		// map containing the number of groups and the different cells in each group
		map<int,vector<Cell*>> groups;

	public:
		// puzzle constructor with the default puzzle being a 3x3 puzzle 
		Sudoku(int r = 3, int c = 3, int gw = 3, int gh = 3 ){
			rows = r; gwidth = gw;
			cols = c; gheight = gh;
			
			// create all the cell in every row and column
			for(int y = 0; y < rows; y++){
				for(int x = 0; x < cols; x++){
					int n = calcGroup(x,y); 
					Cell* A = graph.addCell(x,y,n);

					groups[n].push_back(A); // add each cell to their respective group
				}
			}
		}
		
		// Method to calculate each cells group number based on the group height and width and the number of rows and columns
		int calcGroup(int x, int y){
			int colGroups = cols/gwidth;
			return x/gwidth + colGroups * (y/gheight);
		}
		
		// Method to set the value (i.e. the number stored in them) of each cell
		void setVal(int x, int y, int k){
			Cell* B = graph.getCell(x,y);
			B->k = k;
		}
		
		// Method to populat all the cells in the puzzle with their given values if they have one, and 0 if they dont have any.
		void populate(vector<int> input){
			for(int y = 0; y < rows; y++){
				for(int x = 0; x < cols; x++){
					setVal(x,y,input[x+y*cols]);
				}
			}
		}
		
		// Method to get a set of cell pointers to the relations of the input cell
		set<Cell*> getRelatives(Cell* input){
			int x = input -> x;
			int y = input -> y;
			set<Cell*> relatives;
			
			// iterates through every column in the fixed row and gets all the relatives of the input cell, i.e. gets all the cells
			// in the same row, column and group as the input cell. Does the same for every row int the fixed column
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
			relatives.erase(input); // makes sure teh input cell is not included as a relative to itself
			return relatives;
		}
		void hLine(){
			for(int i = 0; i < cols; i++){
				if(i % gwidth == 0) cout << "+";
				cout << "-----------";
			}
			cout << "+\n";
		}
		
		// draws the sudoku puzzle 
		void draw (){
			for(int j = 0; j < rows; j++){
				if(j % gheight == 0) hLine();
				for(int i = 0; i < cols; i++){
					if(i % gwidth == 0) cout << "|";
					
					printf("[%3i] ",j*rows+i+1);

					int k = graph.getCell(i,j)->k;
					
					if( k != 0) printf("\033[1;46;30m %3i \033[0m",k);
					else cout << "  *  ";

					if(i+1 == cols) cout << "|";
				}
				cout << "\n";
			}
			hLine();
		}

		// Method to create edges (Relations) between verices.
		void makeRelations(){
			vector<Cell*> cells = graph.getCellPtrs();
			
			// Iterate through the vector of cells checking for cells that do not have a 0 value stored in them
			for(auto A: cells){
				if (A->k == 0) continue;
				
				// if they dont have a value of 0, then make a connection between it and another cell whose k value is 0 and is not already
				// connected to another cell with the same value
				for(auto B : getRelatives(A)){
					if (B->k > 0) continue;
					bool alreadyConnected = false;

					for(auto r : B->relations){
						alreadyConnected = (r.k == A->k);
						if (alreadyConnected) break;
					}
					
					// if the second cell is already connected to a cell with the same value, then dont make another relation
					if(alreadyConnected) continue;
					graph.addRelation(A,B, A->k); // add a relation between both cells
				}
			}
		}
		
		// Method to get possibility map used to solve the puzzle. Possibility map is a key value map of all the cells and how many solutions they have
		intVecCellMap getPsbltyMap(){
			int kolors = gwidth * gheight;
			vector<Cell*> cells = graph.getCellPtrs();
			intVecCellMap psbltyMap;
			
			// iterate through all the cells with a value of 0 in the puzzle, going through and counting the number of relatives they have and calculating
			// the number of of solutions they have which is gotten by subtracting the number ofrelations they have from the total number of possible relations
			// Add each cell as a value to their respective possibility count key, then return the map.
			for(auto A: cells){
				if (A->k > 0) continue;
				int cnvrgCount = A->relations.size();
				int psbltyCount = kolors - cnvrgCount;
				psbltyMap[psbltyCount].push_back(A);
			}
			return psbltyMap;
		}
				
		// Method to solve/calculate the value of all the cells with only one possible solution.
		void solveOnePsblty(intVecCellMap psbltyMap){
			// iterate through all the cells with only one possible solution and create a set of the weight of all its relations, with the weight being
			// the value of the cell it is related to
			for (auto pCell:psbltyMap[1]){
				if(verbose) cout << "one solution: " << *pCell << "\n";
				set<int> connectedKs;
				for(auto r:pCell->relations){
					connectedKs.insert(r.k);
				}
				// iterate through the weight of all the relations and look for the weight that is not in the set, that will be the value of the cell.
				int newK;
				for(newK = 1; connectedKs.count(newK); newK++);

				if(verbose) cout << "solution: " << newK << "\n";
				pCell->k = newK;
			}

		}
		
		// Method to solve/ calculate the value of the rest of the puzzle i.e. the cells that have more than one possible solution
		void solveLeastPsblty(intVecCellMap psbltyMap, int leastKey){
			// get all the cells with the possible solutions specified in the input.
			vector<Cell*> lPsbltyCells = psbltyMap[leastKey];
			// get the best candidate, by checking which cell has the most number of weighted relatives. Whichever cells that is,
			// is the best candidate to be solved next.
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
			// calculate the value of the cell with the least possibilities in a very similar fashion as the single possibility cell
			// except there are more guesses to be made, because there is more than one possible soluti  on
			set<int> connectedKs;
			for(auto r:lPsbltyCell->relations){
				connectedKs.insert(r.k);
			}
			int newK;
			for(newK = 1; connectedKs.count(newK); newK++);
			if(verbose) cout << "solution: " << newK << "\n";
					lPsbltyCell->k = newK;

		}

		// method that puts together all the previous steps in order to fully solve the sudoku puzzle
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
				// if the possibility map (map of all the cells with no value in them and their possible solutions) is empty,
				// then it means the puzzle has already been solved and the while loop breaks. If the possibility map is filled 
				// with 0 or there is at least an occurence of it, it means there is a cell with no possible solutions, hence
				// the puzzle is unsolvable and break the while loop
				if(psbltyMap.size() == 0){
					solved = true; break;
				} else if ( psbltyMap.count(0) ){
				
					break;
				}

				// check if there are Cells with only 1 possible solution and then solve those cells, then check for the 
				// cells with the next least possible solution and solve it, till the puzzle is solved
				int leastKey = psbltyMap.begin()->first;
				if(verbose) cout << "least posibilities: " << leastKey << "\n";
				if (leastKey == 1){
					solveOnePsblty(psbltyMap);
				} else {
					solveLeastPsblty(psbltyMap, leastKey);
				} 
			}
			if(!solved){	// print an error message if the puzzle is unsolvable
				cout << "PUnsolvable puzzle\n";
			}
		}

};

