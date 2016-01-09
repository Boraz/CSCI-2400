#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include <omp.h>

using namespace std;

#include "rtdsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  }
}


double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output) {
	
long long cycStart, cycStop;

cycStart = rdtscll();

output -> width = input -> width;
output -> height = input -> height; 

const int columns = (input -> width - 1), rows = (input -> height - 1); // sub here , eliminate it inside the loop // all constants ^ up here and most of this, use const
const int divisor = filter -> getDivisor(); // same
int c, p, r;

int data[3][3];
	#pragma omp parallel for
	for (int m = 0; m < 3; m++) {
	data[m][0] = filter -> get(m,0);
	data[m][1] = filter -> get(m,1);
	data[m][2] = filter -> get(m,2);
	}
	
		if (data[1][0] == 1 && data[1][1] == 1 && data[1][2] == 1) {  // average 
			#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += (input -> color[p][r1][c1]);
          acc1 += (input -> color[p][r1][c]);
          acc1 += (input -> color[p][r1][c2]);
          acc2 += (input -> color[p][r][c1]);
          acc2 += (input -> color[p][r][c]);
		  acc2 += (input -> color[p][r][c2]);
          acc3 += (input -> color[p][r2][c1]);
          acc3 += (input -> color[p][r2][c]);
          acc3 += (input -> color[p][r2][c2]);
          
          acc1 += (acc2 + acc3);
          
		acc1 /= divisor;
		
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}
	}
	else if (data[1][0] == 1 && data[1][1] == -7 && data[1][2] == 1) { // edge
		#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += (input -> color[p][r1][c1]);
          acc1 += (input -> color[p][r1][c]);
          acc1 += (input -> color[p][r1][c2]);
          acc2 += (input -> color[p][r][c1]);
          acc2 += -(input -> color[p][r][c] * 7);
		  acc2 += (input -> color[p][r][c2]);
          acc3 += (input -> color[p][r2][c1]);
          acc3 += (input -> color[p][r2][c]);
          acc3 += (input -> color[p][r2][c2]);
          
          acc1 = (acc1 + acc2 + acc3);
       
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}
	}
	else if (data[0][2] == -1 && data[1][2] == -1 && data[2][1] == -1 && data[2][2] == -1) { // emboss
		#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += (input -> color[p][r1][c1]); // 1
          acc1 += (input -> color[p][r1][c]); // 1
          acc1 += -(input -> color[p][r1][c2]);  // -1
          acc2 += (input -> color[p][r][c1]); // 1
          acc2 += (input -> color[p][r][c]); // 1
		  acc2 += -(input -> color[p][r][c2]); // -1
          acc3 += (input -> color[p][r2][c1]);
          acc3 += -(input -> color[p][r2][c]); // -1
          acc3 += -(input -> color[p][r2][c2]); // -1
          
          acc1 += (acc2 + acc3);
          
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}
	}
	else if (data[1][0] == 4 && data[1][1] == 8 && data[1][2] == 4) { // gauss
		#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
           // 0
          acc1 += (input -> color[p][r1][c] * 4); // data[0][1]
           // 0
          acc2 += (input -> color[p][r][c1] * 4); // data[1][0]
          acc2 += (input -> color[p][r][c] * 8); // data[1][1]
		  acc2 += (input -> color[p][r][c2] * 4); // data[1][2]
		   // 0
          acc1 += (input -> color[p][r2][c] * 4); // data[2][1]
           // 0
          acc1 += acc2;
          
          acc1 /= divisor;
          
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}
	}
	else if (data[1][0] == 0 && data[1][1] == 0 && data[1][2] == 0) { // hline
		#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += -(input -> color[p][r1][c1]); // data[0][0]
          acc1 += -(input -> color[p][r1][c] + input -> color[p][r - 1][c]); // data[0][1] (*-2)
          acc1 += -(input -> color[p][r1][c2]); // data[0][2]
          
          acc2 += (input -> color[p][r2][c1]); // data[2][0]
          acc2 += (input -> color[p][r2][c] + input -> color[p][r + 1][c]); // data[2][1] (*2)
          acc2 += (input -> color[p][r2][c2]); // data[2][2]
          
          acc1 += acc2;
          
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}
	}
	else if (data[0][0] == 11 && data[0][1] == 10 && data[0][2] == 1) { // sharpen
	#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += (input -> color[p][r1][c1] * 11);
          acc1 += (input -> color[p][r1][c] * 10);
          acc1 += (input -> color[p][r1][c2]); // data[0][2]
          acc2 += -(input -> color[p][r][c1]); // data[1][0]
          acc2 += -(input -> color[p][r][c]); // data[1][1]
		  acc2 += -(input -> color[p][r][c2]); // data[1][2]
          acc3 += -(input -> color[p][r2][c1]); // data[2][0]
          acc3 += -(input -> color[p][r2][c]); // data[2][1]
          acc3 += -(input -> color[p][r2][c2]); // data[2][2]
          
          acc1 += (acc2 + acc3);
          
          acc1 /= divisor;
		
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}	
	}
	else if (data[1][0] == -2 && data[1][1] == 0 && data[1][2] == 2) { // vline
		#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += -(input -> color[p][r1][c1]); // data[0][0]
           // 0 data[0][1]
          acc1 += (input -> color[p][r1][c2]); // data[0][2]
          acc2 += -(input -> color[p][r][c1] << 1); // data[1][0]
           // 0 data[1][1]
		  acc2 += (input -> color[p][r][c2] << 1); // data[1][2]
          acc3 += -(input -> color[p][r2][c1]); // data[2][0]
           // 0 data[2][1]
          acc3 += (input -> color[p][r2][c2]); // data[2][2]
          
          acc1 += (acc2 + acc3);
          
		acc1 /= divisor;
		
		if ( acc1 < 0 ) {
			output -> color[p][r][c] = 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] = 255;
		}
		else {
		output -> color[p][r][c] = acc1;
		}
		}
	  }
	}
	}
	else {
		#pragma omp parallel for
	for(p = 0; p < 3; p++) {
	  for(r = 1; r < rows; r++) {
		  int r1 = r - 1, r2 = r + 1;
		for(c = 1; c < columns; c++) {
		  int c1 = c - 1, c2 = c + 1;
          register int acc1 = 0, acc2 = 0, acc3 = 0;
          
          acc1 += (input -> color[p][r1][c1] * data[0][0]);
          acc1 += (input -> color[p][r1][c] * data[0][1]);
          acc1 += (input -> color[p][r1][c2] * data[0][2]);
          acc2 += (input -> color[p][r][c1] * data[1][0]);
          acc2 += (input -> color[p][r][c] * data[1][1]);
		  acc2 += (input -> color[p][r][c2] * data[1][2]);
          acc3 += (input -> color[p][r2][c1] * data[2][0]);
          acc3 += (input -> color[p][r2][c] * data[2][1]);
          acc3 += (input -> color[p][r2][c2] * data[2][2]);
          
          acc1 += (acc2 + acc3);
          
		if ( divisor != 1 ) {
			acc1 /= divisor;
		}
		if ( acc1 < 0 ) {
			output -> color[p][r][c] == 0;
		}
		else if ( acc1 > 255 ) { 
			output -> color[p][r][c] == 255;
		}
		else {
		output -> color[p][r][c] = acc1;
	}
		}
	  }
	}
	}

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}
