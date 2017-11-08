#define HAS_VTK 1

/* The Circle class (All source codes in one file) (CircleAIO.cpp) */
#include <iostream>    // using IO functions
#include <string>      // using string
#include "../include/LaShellWallThickness.h"

using namespace std;


LaShellWallThickness::LaShellWallThickness()
{
	_output_la = new LaShell(); 
}

LaShellWallThickness::~LaShellWallThickness()
{

}

void LaShellWallThickness::SetInputData(LaShell* shell)
{
	_source_la = shell; 
}

void LaShellWallThickness::SetInputData2(LaShell* shell)
{
	_target_la = shell; 
}

LaShell* LaShellWallThickness::GetOutput()
{
	return _output_la; 
}

double LaShellWallThickness::GetEuclidean(double* p1, double* p2)
{
	double sum; 
	sum = ((p1[0] - p2[0])*(p1[0] - p2[0])) + ((p1[1] - p2[1])*(p1[1] - p2[1])) + ((p1[2] - p2[2])*(p1[2] - p2[2]));
	return sqrt(sum); 
}

void LaShellWallThickness::Update()
{
	double pN[3], p[3], tolerance = .001;

	// Outputs of vtkModifiedBSPTree 
	double t; // Parametric coordinate of intersection (0 (corresponding to p1) to 1 (corresponding to p2))
	double x[3]; // The coordinate of the intersection
	double pcoords[3];
	int subId;

	// VTK error logging 
	vtkSmartPointer<vtkFileOutputWindow> fileOutputWindow = vtkSmartPointer<vtkFileOutputWindow>::New();
	fileOutputWindow->SetFileName("vtkLog.txt");
	vtkOutputWindow* outputWindow = vtkOutputWindow::GetInstance();
	if (outputWindow)
	{
		outputWindow->SetInstance(fileOutputWindow);
	}

	vtkSmartPointer<vtkPolyData> Source_Poly = vtkSmartPointer<vtkPolyData>::New(); 
	vtkSmartPointer<vtkPolyData> Target_Poly = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> Output_Poly = vtkSmartPointer<vtkPolyData>::New();


	_source_la->GetMesh3D(Source_Poly);
	_target_la->GetMesh3D(Target_Poly);

	// See https://www.vtk.org/Wiki/VTK/Examples/Cxx/DataStructures/ModifiedBSPTree_IntersectWithLine
	// Create the tree
	vtkSmartPointer<vtkModifiedBSPTree> bspTree = vtkSmartPointer<vtkModifiedBSPTree>::New();
	bspTree->SetDataSet(Target_Poly);
	bspTree->BuildLocator();

	vtkSmartPointer<vtkFloatArray> Output_Poly_Scalar = vtkSmartPointer<vtkFloatArray>::New();
	Output_Poly_Scalar->SetNumberOfComponents(1);

	vtkSmartPointer<vtkPolyDataNormals> Source_Poly_Normals = vtkSmartPointer<vtkPolyDataNormals>::New();
	Source_Poly_Normals->ComputeCellNormalsOn();
	Source_Poly_Normals->SetInputData(Source_Poly);
	Source_Poly_Normals->FlipNormalsOn();
	Source_Poly_Normals->Update();

	Output_Poly->DeepCopy(Source_Poly);
	
	vtkSmartPointer<vtkFloatArray> Source_pNormals = vtkFloatArray::SafeDownCast(Source_Poly->GetPointData()->GetNormals());
	
	for (vtkIdType i = 0; i < Source_Poly->GetNumberOfPoints(); ++i) {
		
		Source_Poly->GetPoint(i, p); 
		Source_pNormals->GetTuple(i, pN);

		// find intersection with target 
		vtkIdType iD = bspTree->IntersectWithLine(p, pN, tolerance, t, x, pcoords, subId);

		float distance_to_target = GetEuclidean(p, x); 

		Output_Poly_Scalar->InsertNextTuple1(distance_to_target);
		//cout << i << ", distance = " << distance_to_target << endl; 
		//double this_scalar = Source_Poly_Scalar->GetTuple1(i);
		//_mesh_vertex_values.push_back(this_scalar);
	}
	
	Output_Poly->GetPointData()->SetScalars(Output_Poly_Scalar);
	
	_output_la->SetMesh3D(Output_Poly);

}


