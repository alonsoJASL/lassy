/*
 *  LaVolumeGraphTraversal.h
 *
 *  Sparse graph traversal utility for vtkUnstructuredGrid meshes.
 *
 *  VTK provides vtkDijkstraGraphGeodesicPath for vtkPolyData but has no
 *  equivalent for vtkUnstructuredGrid.  This class fills that gap by
 *  building an adjacency list from cell connectivity and exposing:
 *
 *    - Dijkstra shortest paths between two nodes
 *    - N-order neighbourhood expansion around a node
 *    - Euclidean edge weighting (optional; topology-only by default)
 *
 *  Build() must be called once after SetInputGrid() before any queries.
 *  The graph is invalidated if the grid changes — call Build() again.
 *
 *  Implementation uses std::priority_queue (standard library only,
 *  no additional dependencies).
 */
#pragma once
#define HAS_VTK 1

#include <vector>
#include <utility>
#include <limits>
#include <cmath>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>


class LaVolumeGraphTraversal {

public:

    /*
     * Edge weighting strategy.
     * Euclidean: weight = 3D distance between endpoints.
     * Topological: weight = 1 for every edge (hop count).
     */
    enum class EdgeWeight {
        Euclidean   = 1,
        Topological = 2
    };

    LaVolumeGraphTraversal();
    ~LaVolumeGraphTraversal() = default;

    // ------------------------------------------------------------------
    // Setup
    // ------------------------------------------------------------------

    void SetInputGrid(vtkUnstructuredGrid* grid);

    /*
     * Builds the adjacency list from cell connectivity.
     * Must be called before ShortestPath() or GetNeighboursAroundPoint().
     * Complexity: O(C * V_per_cell) where C = number of cells.
     */
    void Build();

    void SetEdgeWeightToEuclidean();
    void SetEdgeWeightToTopological();

    // ------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------

    /*
     * Returns the ordered list of node IDs on the shortest path from
     * start_id to end_id, inclusive of both endpoints.
     * Returns an empty vector if no path exists or Build() has not
     * been called.
     */
    std::vector<vtkIdType> ShortestPath(vtkIdType start_id,
                                        vtkIdType end_id) const;

    /*
     * Returns all nodes reachable within max_hops topological hops of
     * point_id, paired with their hop depth.
     * Uses BFS — ignores edge weights for neighbourhood expansion.
     */
    std::vector<std::pair<vtkIdType, int>> GetNeighboursAroundPoint(
        vtkIdType point_id,
        int       max_hops) const;

    /*
     * Returns the number of nodes in the graph.
     * 0 if Build() has not been called.
     */
    vtkIdType GetNumberOfNodes() const;

    // ------------------------------------------------------------------
    // Static utilities
    // ------------------------------------------------------------------

    static double Euclidean(const double* a, const double* b);

private:

    vtkUnstructuredGrid* _grid;     // non-owning, caller retains ownership
    EdgeWeight           _weight_mode;
    bool                 _built;

    /*
     * Adjacency list: _adj[i] = list of (neighbour_id, edge_weight) pairs.
     */
    std::vector<std::vector<std::pair<vtkIdType, double>>> _adj;

    vtkIdType _num_nodes;

    /*
     * Extracts the unique edges from a single cell's point list and
     * inserts them into _adj bidirectionally.
     */
    void InsertCellEdges(vtkIdList* point_ids);
};