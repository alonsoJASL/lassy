/*
*      Author:
*      Dr. Rashed Karim
*      Department of Biomedical Engineering, King's College London
*      Email: rashed 'dot' karim @kcl.ac.uk
*      Copyright (c) 2017
*/

/*
*	The LaShellAssignLabels finds gaps within a user-defined path of a binarised data in shell
*   The path is constructed from user-defined points and computing the Djikstra's shortest path between
*   these adjacent points
*
*   Note: It uses the built-in VTK renderer for displaying the mesh and taking user input
*/
#pragma once

#include <vtkAppendFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkConnectivityFilter.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkCellLocator.h>
#include "vtkPolyData.h"
#include <vtkCellArray.h>
#include "vtkGenericCell.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include <vtkCellData.h>
#include <vtkPointLocator.h>
#include <vtkMath.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataReader.h>
#include <vtkPlanes.h>
#include <vtkPointData.h>
#include <vtkThreshold.h>
#include <vtkThresholdPoints.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataWriter.h>
#include <vtkDijkstraGraphGeodesicPath.h>
#include <vtkTriangle.h>
#include <vtkCellDataToPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkSelectPolyData.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkStructuredPoints.h>
#include <vtkTubeFilter.h>
#include <vtkPolyLine.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLineSource.h>
#include <vtkCallbackCommand.h>
#include <vtkCellPicker.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <string>
#include <sstream>
#include <vtkPointPicker.h>

#include "LaShellAlgorithms.h"
#include "LaShell.h"


class LaShellAssignLabels : public LaShellAlgorithms {

protected:
	LaShell* _source_la;

public:

    vtkSmartPointer<vtkCellPicker> _cell_picker;
		vtkSmartPointer<vtkPointPicker> _point_picker;
    vtkSmartPointer<vtkPolyData> _SourcePolyData;
    vtkSmartPointer<vtkRenderWindow> _RenderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> _InteractorRenderWindow;

    std::vector<int> _pointidarray;
		std::vector<int> _assignedlabels;
		std::vector<int> _codearray;
		std::vector<int> _labelsinmesh, _labelsAssigned;

    // Helper Functions
		void SetInputData(LaShell* shell);

    // Static functions
    static void KeyPressEventHandler(vtkObject* obj, unsigned long,void *sr, void *v);
    static void CreateSphere(vtkSmartPointer<vtkRenderer> iren, double radius, double position3D[]);
		static vtkIdType GetFirstCellVertex(vtkPolyData* poly, vtkIdType cellID, double point_xyz[]);
    // Get functions
    void GetConnectedVertices(vtkSmartPointer<vtkPolyData> mesh, int seed, vtkSmartPointer<vtkIdList> connectedVertices);
    vtkSmartPointer<vtkPolyData> GetSourcePolyData();
    vtkSmartPointer<vtkRenderWindowInteractor> GetWindowInteractor();
    vtkSmartPointer<vtkCellPicker> GetCellPicker();

    void Run();

	LaShellAssignLabels();
	~LaShellAssignLabels();

};
