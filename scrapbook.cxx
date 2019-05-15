//get all cells that vertex 'ptid' is a part of
vector<int> cellIDsInCorridor;
vtkSmartPointer<vtkCellArray> cellarr = vtkSmartPointer<vtkCellArray>::New();

for (int ix=0;ix<pointNeighbours.size(); ix++){
  int ptid = pointNeighbours[ix];
  vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
  mesh->GetPointCells(ptid, cellIdList);
}

//loop through all the cells that use the ptid point
for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++){
  cellIDsInCorridor.push_back(cellIdList->GetId(i));
  vtkCell* cell = mesh->GetCell(cellIdList->GetId(i));
  if(cell->GetNumberOfEdges() <= 0){
    continue;
  }

  // Going through the edges of each cell, remember that an edge is
  // made up of two vertices
  for(vtkIdType e = 0; e < cell->GetNumberOfEdges(); e++){
    vtkCell* edge = cell->GetEdge(e);
    vtkIdList* pointIdList = edge->GetPointIds();

    if(pointIdList->GetId(0) == ptid || pointIdList->GetId(1) == ptid)
    {
      if(pointIdList->GetId(0) == ptid)
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
