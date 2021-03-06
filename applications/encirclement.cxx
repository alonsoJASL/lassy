#define HAS_VTK 1

#define DO_MEAN 1
#define DO_MEDIAN 2

#include "LaShellGapsInBinary.h"
#include <numeric>

/*
*      Author:
*      Dr. Rashed Karim
*      Department of Biomedical Engineering, King's College London
*      Email: rashed 'dot' karim @kcl.ac.uk
*      Copyright (c) 2017
*
*	   This application computes the gaps within a user-defined path on a binarised shell
*/
int main(int argc, char * argv[])
{
	char* input_f1, *output_f="";
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

				else if (string(argv[i]) == "-t") {
					fill_threshold = atof(argv[i + 1]);
					foundArgs2 = true;
				}

				else if (string(argv[i]) == "-n") {
					neighbourhood_size = atoi(argv[i + 1]);
					foundArgs2 = true;
				}
				else if (string(argv[i]) == "-o") {
					output_f = argv[i + 1];

				}


			}

		}
	}

	if (!(foundArgs1))
	{
		cerr << "Check your parameters\n\nUsage:"
			"\nExtracts mesh data from a user-defined trajectory on mesh. Mesh data should be Point Scalars (VTK)"
			"\n(Mandatory)\n\t-i <source_mesh_vtk>"
			"\n\n(optional)\n\t-t <the threshold value for determining filling>"
			"\n\t-n <neighbourhood size, default = 3>"
			"\n\t-o <specify output CSV filename, otherwise the name defaults to encircle.csv>"
			"\n\nNote the neighbourhood size n is the n-th order neighbours that are included, adjacent vertices are 1-order neighbours"<< endl;


		exit(1);
	}
	else
	{
		LaShell* source = new LaShell(input_f1);

		LaShellGapsInBinary* application = new LaShellGapsInBinary();

		if (neighbourhood_size > 0){
			application->SetNeighbourhoodSize(neighbourhood_size);
		}

		if(fill_threshold > 0){
			application->SetFillThreshold(fill_threshold);
		}

		if (strlen(output_f) > 0) {
			application->SetOutputFileName(output_f);
		}

		application->SetInputData(source);

		cout << "Waiting for you to pick points on the mesh to draw a line, \nor I could complete a circle from your picked points"
		"\n - Press x on keyboard for picking points on the mesh\n - Press l for drawing a line between your points and extract data\n - Press c to draw circle between points and extract data\n\n";
		application->Run();



	}

}
