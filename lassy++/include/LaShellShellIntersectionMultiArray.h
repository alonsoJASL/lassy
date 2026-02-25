/*
*      Author:
*      Dr. Rashed Karim
*      Department of Biomedical Engineering, King's College London
*      Email: rashed 'dot' karim @kcl.ac.uk
*      Copyright (c) 2017
*/

/*
*	The LaShellShellIntersectionMultiArray class copies multi-array (i.e. multiple) scalars from one vtk to other, using two separate methods
*
*/
#pragma once
#define HAS_VTK 1

enum IntersectionMappingMethod {
	CopyUsingPointId = 1, 
	CopyUsingNormal = 2
}; 


#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkPolyDataReader.h>
#include <vtkFileOutputWindow.h>
#include <vtkPolyDataWriter.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkModifiedBSPTree.h>
#include <vtkPointLocator.h>
#include <string>

#include "LaShellAlgorithms.h"
#include "LaShellShellIntersection.h"
#include "MathBox.h"


; 


class LaShellShellIntersectionMultiArray : public LaShellShellIntersection {


private:
	//  Inherited from LaShellShellIntersection

	IntersectionMappingMethod _copy_method;

protected:
	static double GetEuclidean(double* p1, double* p2); 
	static void GetFiniteLine(double* start, double* direction, double max_distance, double which_direction, double* end);
	
		
public:
	LaShellShellIntersectionMultiArray();
	~LaShellShellIntersectionMultiArray();


	static LaShellShellIntersectionMultiArray *New();
	
	void SetInputData(LaShell* shell);			// source
	void SetInputData2(LaShell* shell);			// target 
	
	void SetDirectionToOppositeNormal();

	void SetCopyScalarsUsingPointid();
	void SetCopyScalarsUsingNormal();
	void SetDefaultMappingValue(double);

	void Update();

	LaShell* GetOutput();

	
}; 