void LaShellGapsInBinary::Run()
{
	double max_scalar=-1, min_scalar=1e9, s;
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(_SourcePolyData);

	/////////////////////////////////////////Code for Hue-Color Values////////////////////////////////////////////////////////////////
	// first you find what is the maximum and minimum scalar
	vtkFloatArray *scalars = vtkFloatArray::New();
	scalars = vtkFloatArray::SafeDownCast(_SourcePolyData->GetPointData()->GetScalars());  // bring al lthe scalars to an array

	for (vtkIdType i=0;i<_SourcePolyData->GetNumberOfPoints();i++)		// running through each point i
	{
		s = scalars->GetTuple1(i);
		if (s > max_scalar)
			max_scalar = s;

		if (s < min_scalar)
			min_scalar = s;
	}
	std::cout << "Maximum (" << max_scalar << "), and minimum (" << min_scalar
			 << ") scalars " << std::endl;
	// this is your polydata mapper object
	mapper->SetScalarRange(min_scalar, max_scalar);      // you must tell your mapper and lookuptable what is the range of scalars first
	mapper->SetScalarModeToUsePointData();    // mapper->SetScalarModeToUsePointData(); is also possible if you are using cell data
	mapper->ScalarVisibilityOn();
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	lut->SetTableRange(min_scalar, max_scalar);
	lut->SetHueRange(0.6, 0.0);  // this is the way you tell which colors you want to be displayed.
	lut->Build();     // this is important
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

	/*
	_cell_picker = vtkSmartPointer<vtkCellPicker>::New();
	_cell_picker->SetTolerance(0.0005);	// You set a tolerance meaning to what degree of accuracy it is able to select points
	_InteractorRenderWindow->SetPicker(_cell_picker);		// and you tell which interactor this cell picker is part of (you only have one interactor)
	*/
	// point pickers
	_cell_picker = vtkSmartPointer<vtkCellPicker>::New();
	_cell_picker->SetTolerance(0.005);
	_InteractorRenderWindow->SetPicker(_cell_picker);

	vtkCallbackCommand *callback = vtkCallbackCommand::New();
  callback->SetCallback(LaShellGapsInBinary::KeyPressEventHandler);
	callback->SetClientData(this);

	// invoke callback when key is pressed
	_InteractorRenderWindow->AddObserver(vtkCommand::KeyPressEvent, callback);
	_InteractorRenderWindow->Start();


}
