all: Main.cpp GC.cpp
	g++ -g -o allocator_test Main.cpp GC.cpp
