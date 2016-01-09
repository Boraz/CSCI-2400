##
##

CXX	=g++
##
## Use our standard compiler flags for the course...
##
CXXFLAGS= -m64 -g -O2 -fno-omit-frame-pointer

goals: judge
	echo "Done"

filter: FilterMain.cpp Filter.cpp cs1300bmp.cc
	$(CXX) $(CXXFLAGS) -o filter FilterMain.cpp Filter.cpp cs1300bmp.cc
##
## Parameters for the test run
##
FILTERS = gauss.filter vline.filter hline.filter emboss.filter
IMAGES = boats.bmp blocks-small.bmp
TRIALS = 1 2 3 4

judge: filter
	-./Judge -p ./filter -i boats.bmp
	-./Judge -p ./filter -i blocks-small.bmp

valgrind: filter
valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter gausss.filter blocks-small.bmp

valgrind: filter
valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter gausss.filter boats.bmp

valgrind: filter
valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter sharpen.filter blocks-small.bmp

valgrind: filter
valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter sharpen.filter boats.bmp

valgrind: filter
valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter avg.filter blocks-small.bmp

valgrind: filter
valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter avg.filter boats.bmp

clean:
	-rm *.o
	-rm filter
	-rm filtered-*.bmp
