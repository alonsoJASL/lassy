/*
 *  LaShellSyntheticScar.h
 *
 *  Generates a synthetic scar scalar field on a vtkPolyData mesh.
 *
 *  A set of seed point IDs defines a closed path on the mesh surface.
 *  Dijkstra shortest paths are computed between consecutive seeds (closing
 *  the loop), producing a corridor of path vertices.  Every mesh vertex
 *  within the neighbourhood radius accumulates a distance-weighted
 *  contribution from each nearby path vertex.  The chosen falloff kernel
 *  (Gaussian or Linear) maps distance to intensity in [0, 1].  Because
 *  contributions are summed rather than maximised, regions where two path
 *  segments converge will exhibit higher intensity naturally.
 *
 *  The output scalar array is added to the mesh under a unique name
 *  ("synthetic_scar", "synthetic_scar_1", "synthetic_scar_2", ...) so
 *  existing arrays are never overwritten.  Multiple calls on the same mesh
 *  can therefore layer independent scar regions.
 *
 *  Input mesh must be vtkPolyData.  UnstructuredGrid support is deferred
 *  to a future pass — use the ugrid2vtk application to convert beforehand.
 */
#pragma once

#include <string>
#include <vector>
#include <cmath>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkDijkstraGraphGeodesicPath.h>
#include <vtkIdList.h>
#include <vtkPointLocator.h>
#include <vtkMath.h>

#include "LaShellAlgorithms.h"
#include "LaShell.h"

enum class FalloffKernel : int {
    Gaussian = 1,
    Linear = 2
};

class LaShellSyntheticScar : public LaShellAlgorithms {

private:
    LaShell *_source_la;
    LaShell *_output_la;

    vtkSmartPointer<vtkPolyData> _source_poly;

    std::vector<int> _seed_point_ids;

    int _neighbourhood_size;        // N-order neighbourhood for spread
    double _sigma;                  // Gaussian σ (default: neighbourhood_size / 2.0)
    FalloffKernel _falloff;         // kernel choice
    std::string _output_array_name; // base name, auto-suffixed when taken

    // visited list used during recursive neighbourhood traversal —
    // same pattern as LaShellGapsInBinary
    std::vector<std::pair<vtkIdType, int>> _visited_point_list;

    // ----------------------------------------------------------------
    // Internal helpers
    // ----------------------------------------------------------------

    /*
     * Recursively collects N-order neighbours of pointId into
     * _visited_point_list.  Mirrors LaShellGapsInBinary::RecursivePointNeighbours
     * exactly so behaviour is consistent with the rest of the library.
     */
    void RecursivePointNeighbours(vtkIdType point_id, int order);

    /*
     * Fills connectedVertices with the 1-ring neighbours of seed on mesh.
     * Direct port of LaShellGapsInBinary::GetConnectedVertices.
     */
    void GetConnectedVertices(vtkSmartPointer<vtkPolyData> mesh,
                              int seed,
                              vtkSmartPointer<vtkIdList> connected_vertices);

    /*
     * Returns all vertices reachable within _neighbourhood_size hops of
     * point_id, together with their traversal depth.
     */
    void GetNeighboursAroundPoint(int point_id,
                                  std::vector<std::pair<int, int>> &neighbours);

    /*
     * Builds Dijkstra paths between consecutive seeds, closing the loop
     * back to seed[0].  Returns all vertex IDs lying on any path segment,
     * deduplicated via a map.
     */
    std::vector<vtkIdType> BuildPathVertices();

    /*
     * Euclidean distance between two 3D points stored as double[3] arrays.
     */
    static double Euclidean(const double *a, const double *b);

    /*
     * Evaluates the chosen falloff kernel.
     *   d         — distance from corridor point to nearest path vertex
     *   max_d     — normalisation distance (neighbourhood radius in world units)
     * Returns a value in [0, 1]; 1 at d == 0, decaying to 0 at d == max_d.
     */
    double EvaluateFalloff(double d, double max_d) const;

    /*
     * Scans existing point data arrays on poly for a name collision with
     * candidate.  If found, appends _1, _2, ... until the name is unique.
     */
    static std::string UniqueArrayName(vtkPolyData *poly,
                                       const std::string &candidate);

public:
    // ----------------------------------------------------------------
    // Pipeline interface  (mirrors LaShellAlgorithms convention)
    // ----------------------------------------------------------------

    /*
     * Input mesh.  Must be a vtkPolyData-backed LaShell.
     */
    void SetInputData(LaShell *shell);

    /*
     * Seed point IDs defining the path boundary.
     * Paths are computed between consecutive IDs; the last ID connects
     * back to the first to close the loop.
     * Minimum 2 IDs required.
     */
    void SetPointIDList(const std::vector<int> &ids);

    /*
     * Reads seed point IDs from a plain-text file, one integer per line.
     * This is the same format that encirclement writes when the user
     * presses 's' in the interactive viewer.
     */
    void ReadPointIDFile(const char *filename);

    /*
     * Number of neighbourhood hops used to expand the corridor around
     * each path vertex.  Larger values produce a wider scar.
     * Default: 3.
     */
    void SetNeighbourhoodSize(int n);

    /*
     * Gaussian σ expressed in world-space units.
     * Controls how steeply intensity falls off with distance.
     * Default: neighbourhood_size / 2.0, recomputed on each Update() call
     * unless SetSigma() has been called explicitly.
     */
    void SetSigma(double sigma);

    /*
     * Select Gaussian falloff  exp(-d² / 2σ²).
     * This is the default.
     */
    void SetFalloffToGaussian();

    /*
     * Select linear falloff  1 - (d / max_d), clamped to [0, 1].
     */
    void SetFalloffToLinear();

    /*
     * Override the default output array name ("synthetic_scar").
     * A numeric suffix is still appended automatically if the name
     * already exists on the mesh.
     */
    void SetOutputArrayName(const char *name);

    void Update();

    LaShell *GetOutput();

    LaShellSyntheticScar();
    ~LaShellSyntheticScar();
};