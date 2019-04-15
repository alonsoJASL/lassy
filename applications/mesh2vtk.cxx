#define HAS_VTK 1

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include <numeric>
using namespace std;

/*
*      Author:
*      Dr. José Alonso Solís-Lemus
*      Department of Biomedical Engineering, King's College London
*      Email: jose_alonso 'dot' solis_lemus @kcl.ac.uk
*      Copyright (c) 2019
*
*    This application converts a shell mesh, where scalar values are stored in
*    Cell data, this converts them into point data as most routines in this
*    library use it.
*
*/
int main(int argc, char * argv[]){
	char* input_csv_fn, *output_fn;
	bool convert=true;
	bool foundArgs1 = false, foundArgs2 = false;

	if (argc >= 1){
		for (int i = 1; i < argc; i++) {
			if (i + 1 != argc) {
				if (string(argv[i]) == "-vtk"){
					input_csv_fn = argv[i + 1];
					foundArgs1 = true;
				}

				else if (string(argv[i]) == "-o"){
					output_fn = argv[i + 1];
					foundArgs2 = true;
				}
		  }
    }
	}

	if (!(foundArgs1 && foundArgs2)){
		cerr << "Check your parameters\n\nUsage:"
			"\nVTK mesh turned into vtkPolyData"
			"\n(Mandatory)\n\t-vtk <vtk file with mesh> \n\t-o <output filename>"
       << endl;

		exit(1);
	}

  cout<<"Success!"<<endl;
}   // end function
