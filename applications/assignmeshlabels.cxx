#define HAS_VTK 1

#define DO_MEAN 1
#define DO_MEDIAN 2

#include <numeric>
#include <iostream>
#include <string>
#include <vector>

#include "LaShellAssignLabels.h"
/*
*      Author:
*      Dr. Jose Alonso solis-Lemums
*      Department of Biomedical Engineering, King's College London
*      Email: jose_alonso 'dot' solis_lemus @kcl.ac.uk
*      Copyright (c) 2019
*
*	   This application computes the gaps within a user-defined path on a
*    binarised shell.
*/
int main(int argc, char * argv[])
{
	char* input_f1, *output_f="", *pointidlist_f="", *output_shell="";
	double fill_threshold = 0.5;
	int neighbourhood_size = 3;

	bool foundArgs1 = false, foundArgs2 = false;

	if (argc >= 1)
	{
		for (int i = 1; i < argc; i++) {
			if (i + 1 != argc) {
				if (string(argv[i]) == "-i") {
					input_f1 = argv[i + 1];
					foundArgs1 = true;
				}
			}

		}
	}

	if (!(foundArgs1))
	{
		cerr << "Check your parameters\n\nUsage:"
			"\nExtracts mesh data from a user-defined trajectory on mesh. "
			" Mesh data should be Point Scalars (VTK).\n Convert your Cell-Scalar meshes with ./mesh2vtk binary."
			"\n(Mandatory)\n\t-i <source_mesh_vtk>"
			<< endl;
		exit(1);
	}
	else{
		LaShell* source = new LaShell(input_f1);
		LaShellAssignLabels* application = new LaShellAssignLabels();
		application->SetInputData(source);

		cout << "input data OK..." << endl;

		cout << "Waiting for you to pick points on the mesh select the corresponding, \n"
		"sections."
		"\n - Press l for LV_endo; Press L for LV_epi"
		"\n - Press r for RV_endo; Press R for RV_epi"
		"\n - Press a for Apex"
		"\n - Press b for Base\n"
		"\n - Press x on keyboard for picking points on the mesh without label (just for tests)"
		"\n - Press m on keyboard to check if you've picked all the labels.\n"
		"\n - Press s to SAVE your selected points for easier access!\n\n";;
		application->Run();

	}
}
