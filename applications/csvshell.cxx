#define HAS_VTK 1


#include "LaShellPointsCSV.h"


/*
*      Author:
*      Dr. Rashed Karim
*      Department of Biomedical Engineering, King's College London
*      Email: rashed 'dot' karim @kcl.ac.uk
*      Copyright (c) 2017
*
*	   This application demonstrates the LaShellPointsCSV class reads a CSV file containing 3D points and scalar values.
*      The scalars are appplied at these 3D locations on a LA mesh
*/
int main(int argc, char * argv[])
{
	char* input_img, *input_csv, *input_f2, *output_vtk;
    char* scalar_array_name = "new_scalar";
    int nn = 5;     // locate mesh vertex neighbours within a certain radius
    int scaling_factor = 1;

	bool foundArgs1 = false;
    bool foundArgs2 = false;
    bool foundArgs3 = false;
    int method = POINT_COPY;

	if (argc >= 1)
	{
		for (int i = 1; i < argc; i++) {
			if (i + 1 != argc) {
				if (string(argv[i]) == "-vtk") {
					input_img = argv[i + 1];
					foundArgs1 = true;
				}

				else if (string(argv[i]) == "-csv") {
					input_csv = argv[i + 1];
                    foundArgs2 = true; 

				}
                else if (string(argv[i]) == "-out") {
					output_vtk = argv[i + 1];
                    

				}

                else if (string(argv[i]) == "-method") {
					method = atoi(argv[i + 1]);
                    foundArgs3 = true; 

				}

                else if (string(argv[i]) == "-nn") {
					nn = atoi(argv[i + 1]);

				}

                else if (string(argv[i]) == "-scaling") {
					scaling_factor = atoi(argv[i + 1]);

				}

                else if (string(argv[i]) == "-scalarname") {
					scalar_array_name = argv[i + 1];

				}
			}

		}
	}

	if (!(foundArgs1 && foundArgs2))
	{
		cerr << "Cheeck your parameters\n\nUsage:"
			"\nReads a CSV file containing 3D points and scalar values. The scalars are appplied at these 3D locations on a LA mesh "
			"\n(Mandatory)\n\t-vtk <input shell> \n\t-csv <csv file>\n\n(Optional)\n\t-method <1=Point copy, 2=Neighbour copy>" 
            "\n\t-nn <neighbours within a certain radius default 5 mm> " << endl;
			
		exit(1);
	}
	else
	{
		LaShell* la_mesh = new LaShell(input_img);
        LaShell* mesh_out = new LaShell();
		
		LaShellPointsCSV* algorithm = new LaShellPointsCSV();
		
        algorithm->SetInputData(la_mesh);
        algorithm->SetScalingFactor(scaling_factor);
		algorithm->ReadCSVFile(input_csv);
        algorithm->SetArrayName(scalar_array_name);
        

		switch (method)
		{
			case POINT_COPY:
				algorithm->SetCopyMethodToPointCopy();
                algorithm->InsertScalarData();
            
                cout << "\nPoint copy method: copies scalars to individual points" << endl; 
				break;

            case NEIGHBOUR_COPY: 
                algorithm->SetCopyMethodToPointCopy();
                algorithm->SetNeighbourRadius(nn);
                algorithm->LocateNeighboursOfPoints();
                algorithm->InsertScalarData();
                cout << "\nPoint copy method: copies scalars to neighbouring points around matched point" << endl; 
                break; 
			
		}

		algorithm->Update();
        algorithm->VisualisePoints();

        mesh_out = algorithm->GetOutput();
		
		mesh_out->ExportVTK(output_vtk);
        
       
	}

}