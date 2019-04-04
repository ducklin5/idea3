server: src/server.cpp
	g++ -std=c++11 src/server.cpp `pkg-config --libs --cflags tesseract opencv` -o server

digraph: src/digraph.cpp
	g++ -std=c++11 src/digraph.cpp -o digraph

sudoku: src/Sudoku.cpp
	g++ -std=c++11 src/Sudoku.cpp -o Sudoku

printer: obj/sudokuprinter.o obj/serialport.o
	g++ -std=c++11 obj/sudokuprinter.o obj/serialport.o -o sudokuprinter
	
obj/sudokuprinter.o: src/sudokuPrinter.cpp
	g++ -std=c++11 -c src/sudokuPrinter.cpp -o obj/sudokuprinter.o

obj/serialport.o: src/serialport.cpp
	g++ -std=c++11 -c src/serialport.cpp -o obj/serialport.o

