#define HAS_VTK 1

#define DO_MEAN 1
#define DO_MEDIAN 2

#include <numeric>
#include <iostream>
#include <string>
#include <vector>

#include "LaShellGapsInBinary.h"
/*
*      Author:
*      Dr. Jose Alonso solis-Lemums
*      Department of Biomedical Engineering, King's College London
*      Email: jose 'dot' solislemus @kcl.ac.uk
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

	bool foundArgs1 = false, foundArgs2 = false, foundArgs3 = false;

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
				else if(string(argv[i]) == "-l"){
					pointidlist_f = argv[i + 1];
					foundArgs3 = true;
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
			"\n\n(optional)"
			"\n\t-l <list with point IDs for this shell. If empty, output pointID.txt is generated>"
			"\n\t-t <the threshold value for determining filling>"
			"\n\t-n <neighbourhood size, default = 3>"
			"\n\t-o <specify output CSV filename, otherwise the name defaults to encircle.csv>"
			"\n\nNote the neighbourhood size n is the n-th order neighbours that are included, adjacent vertices are 1-order neighbours"<< endl;
		exit(1);
	}
	else{
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
		cout << "input data OK..." << endl;

		if(!(foundArgs3)){
				cout << "Waiting for you to pick points on the mesh to draw a line, \n"
				"or I could complete a circle from your picked points"
				"\n - Press x on keyboard for picking points on the mesh"
				"\n - Press l for drawing a line between your points and extract data"
				"\n - Press c to draw circle between points and extract data"
				"\n - Press s to SAVE your selected points for easier access!\n\n";;
				application->Run();
		}
		else{
			cout << "Reading from file: " <<  pointidlist_f << ", and generating an "
			"exploration corridor with a \nneighbourhood depth of " << neighbourhood_size
			<< " and threshold " << fill_threshold << endl;

			vector<int> pointsIDlist;
			int auxID;
			ifstream infile;
			string line;
			stringstream ss;
			ss << "pointsIDlist.txt";
			infile.open(ss.str().c_str(), std::ios_base::in);

			if (infile.is_open()){
				while (infile >> auxID){
					pointsIDlist.push_back(auxID);
				}
				infile.close();
				cout << "File: pointsIDlist.txt read. Calculating." << endl;

				for (int ix=0; ix<pointsIDlist.size(); ix++){
					cout << pointsIDlist[ix] << endl;
				}
				// Run routine
				application->CorridorFromPointList(pointsIDlist);
			}
			else {
				cout << "Unable to open file" << endl;
			}
		}
	}
}
