#define HAS_VTK 1


/* The Circle class (All source codes in one file) (CircleAIO.cpp) */
#include <iostream>    // using IO functions
#include <string>      // using string
#include "../include/LaShellAssignLabels.h"

;


LaShellAssignLabels::LaShellAssignLabels(){
	_source_la = new LaShell();
	_SourcePolyData = vtkSmartPointer<vtkPolyData>::New();

}

LaShellAssignLabels::~LaShellAssignLabels() {
}

void LaShellAssignLabels::SetInputData(LaShell* shell) {
	_source_la = shell;
	_source_la->GetMesh3D(_SourcePolyData);
}

vtkSmartPointer<vtkRenderWindowInteractor> LaShellAssignLabels::GetWindowInteractor(){
	return _InteractorRenderWindow;
}

vtkSmartPointer<vtkPolyData> LaShellAssignLabels::GetSourcePolyData(){
	return _SourcePolyData;
}

vtkSmartPointer<vtkCellPicker> LaShellAssignLabels::GetCellPicker(){
	return _cell_picker;
}

/*
*	Get N-order neighbours of a vertex
*/
void LaShellAssignLabels::GetConnectedVertices(vtkSmartPointer<vtkPolyData> mesh, int seed, vtkSmartPointer<vtkIdList> connectedVertices)
{

  //get all cells that vertex 'seed' is a part of
  vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
  mesh->GetPointCells(seed, cellIdList);

  //std::cout << "There are " << cellIdList->GetNumberOfIds() << " cells that use point " << seed << std::endl;

  //loop through all the cells that use the seed point
	for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
	{

		vtkCell* cell = mesh->GetCell(cellIdList->GetId(i));
		//std::cout << "The cell has " << cell->GetNumberOfEdges() << " edges." << std::endl;

		//if the cell doesn't have any edges, it is a line
		if(cell->GetNumberOfEdges() <= 0)
		{
			continue;
		}

		// Going through the edges of each cell, remember that an edge is
		// made up of two vertices
		for(vtkIdType e = 0; e < cell->GetNumberOfEdges(); e++)
		{
			vtkCell* edge = cell->GetEdge(e);

			vtkIdList* pointIdList = edge->GetPointIds();
			//std::cout << "This cell uses " << pointIdList->GetNumberOfIds() << " points" << std::endl;

			if(pointIdList->GetId(0) == seed || pointIdList->GetId(1) == seed)
			{
				if(pointIdList->GetId(0) == seed)
				 {
				  connectedVertices->InsertNextId(pointIdList->GetId(1));
				 }
				else
				  {
				  connectedVertices->InsertNextId(pointIdList->GetId(0));
				  }
			}
		}
	}
   // std::cout << "There are " << connectedVertices->GetNumberOfIds() << " points connected to point " << seed << std::endl;
}


// A cell has three vertex. Cell is a polygon within a mesh. You input the mesh, the id of the cell/polygon
// and this function returns the 3D position of one of the cell's vertex (as point_xyz)
// and the point ID (as function return)
vtkIdType LaShellAssignLabels::GetFirstCellVertex(vtkPolyData* poly, vtkIdType cellID, double point_xyz[])
{
	vtkIdType vertID;
	vtkCell* cell;
	cell = poly->GetCell(cellID);

	vtkSmartPointer<vtkIdList> cell_vert = vtkSmartPointer<vtkIdList>::New();
	poly->GetCellPoints(cellID, cell_vert);

	vertID = cell_vert->GetId(0);
	poly->GetPoint(vertID, point_xyz);
	return vertID;
}

/*
*	This will handle the event when a user presses the 'x' on the keyboard
*/
void LaShellAssignLabels::KeyPressEventHandler(vtkObject* obj, unsigned long eventId,
                                      void* clientdata, void* vtkNotUsed(calldata)){
	/* Remember you are marking your pick position with sphere.
	First step is to pick and then to place a sphere in the cell you have
	picked_pointidarray

	The code below picks a cell, and you mark one of the cell's vertex with a sphere
	*/
	double screenX, screenY;		// where in the screen you wil be clicking
	vtkIdType cellID, pointID=-1;		// to store cellID of the picked cell

	LaShellAssignLabels* this_class_obj = reinterpret_cast<LaShellAssignLabels*>(clientdata);

	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	vtkSmartPointer<vtkRenderWindow> renderWin = iren->GetRenderWindow();
	vtkSmartPointer<vtkRendererCollection> rendererCollection = renderWin->GetRenderers();
	vtkSmartPointer<vtkRenderer> renderer = rendererCollection->GetFirstRenderer();

	vtkSmartPointer<vtkPolyData> poly_data = vtkSmartPointer<vtkPolyData>::New();
	poly_data = this_class_obj->GetSourcePolyData();

	char keypressed = iren->GetKeyCode();
	bool isLV = keypressed =='l' || keypressed=='L';
	bool isRV = keypressed =='r' || keypressed=='R';
	bool isBa = keypressed =='b' || keypressed=='B';
	bool isAp = keypressed =='a' || keypressed=='A';
	bool isEpi = keypressed=='L' || keypressed=='R';

	int sectionCode;
	if(isLV)
		sectionCode = 2;
	else if(isRV)
		sectionCode = 3;
	else if(isAp)
		sectionCode = 5;
	else if(isBa)
		sectionCode = 7;

	sectionCode += isEpi ? 10 : 0;

	// x is for picking any points - saves: pointID, label
	if (iren->GetKeyCode()=='x'){
		double *pick_position = new double[3];

		// get the x and y co-ordinates on the screen where you have hit 'x'
		screenX = iren->GetEventPosition()[0];
		screenY = iren->GetEventPosition()[1];

		vtkSmartPointer<vtkCellPicker> cell_picker = this_class_obj->_cell_picker;

		cell_picker->Pick(screenX, screenY, 0.0, renderer);
		cellID = cell_picker->GetCellId();
		cell_picker->GetPickPosition(pick_position);

		pointID = LaShellAssignLabels::GetFirstCellVertex(poly_data, cellID, pick_position);
		double picked_scalar = poly_data->GetPointData()->GetScalars()->GetTuple1(pointID);

		std::cout << "Point id picked = " << pointID << " with value:" << picked_scalar
			<< " and co-ordinates of its position = "
			<< pick_position[0] << ", " << pick_position[1] << "," << pick_position[2]
			<< ")\n";

		LaShellAssignLabels::CreateSphere(renderer, 3.5, pick_position);

		iren->Render();
		delete[] pick_position;
	}
	else if (isLV || isRV || isAp || isBa){
		char key = iren->GetKeyCode();
		double *pick_position = new double[3];

		// get the x and y co-ordinates on the screen where you have hit 'x'
		screenX = iren->GetEventPosition()[0];
		screenY = iren->GetEventPosition()[1];

		vtkSmartPointer<vtkCellPicker> cell_picker = this_class_obj->_cell_picker;

		cell_picker->Pick(screenX, screenY, 0.0, renderer);
		cellID = cell_picker->GetCellId();
		cell_picker->GetPickPosition(pick_position);

		pointID = LaShellAssignLabels::GetFirstCellVertex(poly_data, cellID, pick_position);
		double picked_scalar = poly_data->GetPointData()->GetScalars()->GetTuple1(pointID);

		this_class_obj->_pointidarray.push_back(pointID);
		this_class_obj->_assignedlabels.push_back(picked_scalar);
		this_class_obj->_labelsAssigned.at(picked_scalar-1) = 1;
		this_class_obj->_codearray.push_back(sectionCode);

		std::cout << "Key pressed = [" << key << "] with label: " << picked_scalar
			<< " and code:" << sectionCode
			<< "\n";

		LaShellAssignLabels::CreateSphere(renderer, 3.5, pick_position);

		iren->Render();
		delete[] pick_position;
	}
	else if (iren->GetKeyCode()=='s'){
		std::ofstream out_ID, out_CSV;
		std::stringstream ss_ID, ss_CSV;
		int lim = this_class_obj->_pointidarray.size();

		ss_ID << "pointsIDlist.txt";
		ss_CSV << "labelsNcodeslist.txt";
		out_ID.open(ss_ID.str().c_str(), std::ios_base::trunc);
		out_CSV.open(ss_CSV.str().c_str(), std::ios_base::trunc);
		for(int ix=0;ix<lim;ix++){
			out_ID  << this_class_obj->_pointidarray[ix] << std::endl;
			out_CSV << this_class_obj->_assignedlabels[ix] << " "
							<< this_class_obj->_codearray[ix] << std::endl;
		}
		out_ID.close();
		out_CSV.close();

		std::cout << "File: pointIDList.txt created. You can exit the application." << std::endl;
	}
	else if(iren->GetKeyCode()=='m'){ //look for missing labels
		int N = this_class_obj->_labelsinmesh.size();
		for(int ix=0; ix<N; ix++){
			std::cout << this_class_obj->_labelsinmesh[ix] << " "
					 << this_class_obj->_labelsAssigned[ix] << std::endl;
		}
	}
	else if(iren->GetKeyCode()=='d'){
		int N = this_class_obj->_labelsinmesh.size();
		for(int ix=0; ix<N; ix++){
			this_class_obj->_labelsAssigned[ix] = 0;
		}
	}

	//delete [] pick_position;

}

// this wil draw a sphere of a given radus to the renderer
void LaShellAssignLabels::CreateSphere(vtkSmartPointer<vtkRenderer> renderer, double radius, double position3D[])
{
	vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
	sphere->SetThetaResolution(8);
	sphere->SetPhiResolution(8);
	sphere->SetRadius(radius);

	vtkSmartPointer<vtkPolyDataMapper> sphereMapper =vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->ScalarVisibilityOff();
	sphereMapper->SetInputData(sphere->GetOutput());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(sphereMapper);
	actor->GetProperty()->SetColor(0,0,1);
	actor->SetPosition(position3D);
	renderer->AddActor(actor);

}

void LaShellAssignLabels::Run(){
	double max_scalar=-1, min_scalar=1e9, s;
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(_SourcePolyData);

	///////////////////////Code for Hue-Color Values////////////////////////////////
	// first you find what is the maximum and minimum scalar
	vtkFloatArray *scalars = vtkFloatArray::New();
	scalars = vtkFloatArray::SafeDownCast(_SourcePolyData->GetPointData()->GetScalars());

	for (vtkIdType i=0;i<_SourcePolyData->GetNumberOfPoints();i++){
		s = scalars->GetTuple1(i);
		if (s > max_scalar)
			max_scalar = s;

		if (s < min_scalar)
			min_scalar = s;
	}
	std::cout << "Maximum (" << max_scalar << "), and minimum (" << min_scalar
			 << ") scalars " << std::endl;

	for(int ix=min_scalar; ix<=max_scalar; ix++){
		_labelsinmesh.push_back(ix);
		_labelsAssigned.push_back(0);
	}

	mapper->SetScalarRange(min_scalar, max_scalar);
	mapper->SetScalarModeToUsePointData();
	mapper->ScalarVisibilityOn();
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetTableRange(min_scalar, max_scalar);
	lut->SetHueRange(0.65, 0.0);
	lut->Build(); // this is important
	mapper->SetLookupTable(lut);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();

	renderer->AddActor(actor);

	_RenderWindow->AddRenderer(renderer);

	_InteractorRenderWindow = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	_InteractorRenderWindow->SetRenderWindow(_RenderWindow);
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	_InteractorRenderWindow->SetInteractorStyle(interactorStyle);

	_RenderWindow->Render();
	// point pickers
	_cell_picker = vtkSmartPointer<vtkCellPicker>::New();
	_cell_picker->SetTolerance(0.005);
	_InteractorRenderWindow->SetPicker(_cell_picker);

	vtkCallbackCommand *callback = vtkCallbackCommand::New();
  callback->SetCallback(LaShellAssignLabels::KeyPressEventHandler);
	callback->SetClientData(this);

	// invoke callback when key is pressed
	_InteractorRenderWindow->AddObserver(vtkCommand::KeyPressEvent, callback);
	_InteractorRenderWindow->Start();


}
