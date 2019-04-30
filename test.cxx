void LaShellGapsInBinary::getCorridorPoints(vector<vtkSmartPointer<vtkDijkstraGraphGeodesicPath> > allShortestPaths)
{
	typedef map<vtkIdType, int>::iterator it_type;
	map<vtkIdType, int> vertex_ids;
	vector<pair<int, int> > pointNeighbours;
  vector<int> pointIDsInNeighbour;
  int order = _neighbourhood_size;

	for (int i=0;i<allShortestPaths.size();i++){
		vtkIdList* vertices_in_shortest_path = vtkIdList::New();
		vertices_in_shortest_path = allShortestPaths[i]->GetIdList();

		for (int j=0;j<vertices_in_shortest_path->GetNumberOfIds();j++)
			vertex_ids.insert(make_pair(vertices_in_shortest_path->GetId(j),-1));
	}

	for (it_type iterator = vertex_ids.begin(); iterator != vertex_ids.end(); iterator++){
		GetNeighboursAroundPoint2(iterator->first, pointNeighbours, order);

		for (int j=0;j<pointNeighbours.size();j++)
      pointIDsInNeighbour.push_back(pointNeighbours[j].first);

		pointNeighbours.clear();
	}
  this->_corridoridarray = pointIDsInNeighbour;

  ofstream out;
  stringstream ss;
  ss = "CHECK_REPEAT.txt";
  out.open(ss.str().c_str(), std::ios_base::trunc);

  for (int ix=0; ix<this->_corridoridarray.size(); ix++) {
    out << this->_corridoridarray[ix] << endl;
  }
  out.close();
  cout << "Created file: CHECK_REPEAT.txt" << endl;
}
