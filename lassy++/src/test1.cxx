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
	bool isEpi = keypressed=='L' || keypressed=='R';

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

		cout << "Point id picked = " << pointID << " with value:" << picked_scalar
			<< " and co-ordinates of its position = "
			<< pick_position[0] << ", " << pick_position[1] << "," << pick_position[2]
			<< ")\n";

		LaShellAssignLabels::CreateSphere(renderer, 3.5, pick_position);

		iren->Render();
		delete[] pick_position;
	}
	else if (isLV || isRV || isAp || isBa){
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
		this_class_obj->_labelsAssigned.at(picked_scalar-0) = 1;
		this_class_obj->_codearray.push_back(sectionCode);

		cout << "Key pressed = [" << key << "] with label: " << picked_scalar
			<< " and code:" << sectionCode
			<< "\n";

		LaShellAssignLabels::CreateSphere(renderer, 3.5, pick_position);

		iren->Render();
		delete[] pick_position;
	}
	else if (iren->GetKeyCode()=='s'){
		ofstream out_ID, out_CSV;
		stringstream ss_ID, ss_CSV;
		int lim = this_class_obj->_pointidarray.size();

		ss_ID << "pointsIDlist.txt";
		ss_CSV << "labelsNcodeslist.txt";
		out_ID.open(ss_ID.str().c_str(), std::ios_base::trunc);
		out_CSV.open(ss_CSV.str().c_str(), std::ios_base::trunc);
		for(int ix=0;ix<lim;ix++){
			out_ID  << this_class_obj->_pointidarray[ix] << endl;
			out_CSV << this_class_obj->_assignedlabels[ix] << ","
							<< this_class_obj->_codearray[ix] << endl;
		}
		out_ID.close();
		out_CSV.close();

		cout << "File: pointIDList.txt created. You can exit the application." << endl;
	}
	else if(iren->GetKeyCode()=='m'){ //look for missing labels
		int N = this_class_obj->_labelsinmesh.size();
		for(int ix=0; ix<N; ix++){
			cout << this_class_obj->_labelsinmesh[ix] << " "
					 << this_class_obj->_labelsAssigned[ix];
		}
	}

	//delete [] pick_position;

}
