/*
*      Author:
*      Dr. Rashed Karim
*      Department of Biomedical Engineering, King's College London
*      Email: rashed 'dot' karim @kcl.ac.uk
*      Copyright (c) 2017
*/

/*
*	The LaShellAlgorithms is a generalisation for algorithms that work on LaShell objects, i.e. meshes 
*	For more information: https://www.vtk.org/doc/nightly/html/classvtkPolyData.html
*
*/
#pragma once


#include "vtkIdList.h"

#include "LaShell.h"
#include "LaImage.h"
#include "LaImageShellAlgorithms.h"
#include "LaImageNormalInterrogator.h"

/*
*	Interrogates a 3D image along the surface normals of a 3D surface mesh.
*	Projects the interrogated value to the mesh vertex 
*	Note that the interrogation is by default 4 pixels in both directions of the vector
*
*	There is an option to interrogate the image only in the region defined by the mask_img parameter
*/
class LaImageSurfaceNormalAnalysis : public LaImageShellAlgorithms {

private: 
	LaShell* _la_shell;
	LaShell* _normal_step_shell;	
	
	LaImage* _la_binary;
	LaImage* _la_image; 
	LaImage* _mask_image;
	bool _vtk_logging; 
	bool _shell_only_no_mapping;

	double _step_size;

	vtkSmartPointer<vtkPolyData> _mesh_3d; 

	LaImageNormalInterrogator* _normal_interrogate_algorithm;
	 char* _output_fn;
	
public:
	LaShell* _output_shell;
	// Constructor with default values for data members
	/*static LaShellAlgorithms *New();*/
	
	void SetInputDataShell(LaShell* shell); 
	
	void SetInputDataImage(LaImage* img);
	void SetInputDataBinary(LaImage* bin);
	void SetInputDataImageMask(LaImage* mask);

	void SetInputNormalStepShell(LaShell* shell);

	void SetStepSize(double steps);		// defaults to 4 unless called to set 

	void SetVTKLogging();

	void SetAggregationMethodToMax();
	void SetAggregationMethodToMean();
	void SetAggregationMethodToMedian();
	void SetMethodToNoMapping();
	void SetAggregationMethodToIntegral();
	void SetZScoreMean(double);
	void SetZScoreStd(double);

	void SetOutputFileName( char* fn);


	void Update(); 

	LaShell* GetOutput(); 
	
	void SurfaceProjectionOnPoints();




	LaImageSurfaceNormalAnalysis();
	~LaImageSurfaceNormalAnalysis();
	
}; 