#define HAS_VTK 1

#include "LaShell.h"

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include <numeric>
;

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
	char* input_fn, *output_fn;
	bool foundArgs1 = false, foundArgs2 = false;


	if (argc >= 1){
		for (int i = 1; i < argc; i++) {
			if (i + 1 != argc) {
				if (std::string(argv[i]) == "-vtk"){
					input_fn = argv[i + 1];
					foundArgs1 = true;
				}

				else if (std::string(argv[i]) == "-o"){
					output_fn = argv[i + 1];
					foundArgs2 = true;
				}
		  }
    }
	}

	if (!(foundArgs1 && foundArgs2)){
		std::cerr << "Check your parameters\n\nUsage:"
			"\nVTK mesh turned into vtkPolyData"
			"\n(Mandatory)\n\t-vtk <vtk file with mesh> \n\t-o <output filename>"
       << std::endl;

		exit(1);
	}

	std::cout<<"Creating object"<<std::endl;
	LaShell* la = new LaShell(input_fn);
	std::cout<<"Converting to point data."<<std::endl;
	la->ConvertToPointData();
	std::cout<<"Exporting mesh"<<std::endl;
	la->ExportVTK(output_fn);

  std::cout<<"Success!"<<std::endl;
}   // end function
