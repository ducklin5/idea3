#--BUild Flags--------------------------------------
CXX=g++
CXXFLAGS=-std=c++11

#--Directory Variables------------------------------
SRC_DIR=src
OBJ_DIR=obj
dir_guard=@mkdir -p #make sure specified directory exists

srcs = sudokuReader.cpp main.cpp
objs = $(srcs:%.cpp=$(OBJ_DIR)/%.o)
deps = $(SRC_DIR)/Sudoku.cpp
EXE = solver
#--Targets------------------------------------------

all: $(EXE)

$(EXE): $(objs) $(deps)
	$(CXX) $(objs) `pkg-config --libs --cflags tesseract opencv` -o $@ $(CXXFLAGS)


%.o: $(OBJ_DIR)/%.o

#target to build any type of object file in the Obj dir
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(deps)
	$(dir_guard) $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	rm -rf $(OBJ_DIR) $(EXE)
