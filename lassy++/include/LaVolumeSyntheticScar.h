/*
 *  LaVolumeSyntheticScar.h
 *
 *  Generates a synthetic scar scalar field on a volumetric mesh (LaVolume).
 *
 *  Mirrors the public API of LaShellSyntheticScar exactly, with one
 *  addition: SetSurfaceSeeds(), retained as a fallback for callers that
 *  have surface point IDs rather than world coordinates.
 *
 *  Seed input methods:
 *    ReadPointsFile()         — auto-detect or explicit coordinate/ID format
 *    ReadPointCoordinatesFile() — coordinate format, resolved via vtkPointLocator
 *    ReadPointIDFile()          — legacy integer ID format
 *    SetNodeIDList()            — in-memory volumetric node IDs
 *    SetSurfaceSeeds()          — maps surface point IDs to volume nodes
 *
 *  Internally uses LaVolumeGraphTraversal for shortest-path computation
 *  and neighbourhood expansion, replacing vtkDijkstraGraphGeodesicPath
 *  which only operates on vtkPolyData.
 *
 *  Falloff kernels and accumulation behaviour are identical to
 *  LaShellSyntheticScar: contributions are summed (not maximised) so
 *  converging path segments produce higher intensity naturally.
 */
#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <memory>

#include <vtkSmartPointer.h>
#include <vtkPointLocator.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include "LaVolumeAlgorithms.h"
#include "LaVolumeGraphTraversal.h"
#include "LaVolume.h"
#include "LaShell.h"


enum class VolumeFalloffKernel : int {
    Gaussian = 1,
    Linear   = 2
};


class LaVolumeSyntheticScar : public LaVolumeAlgorithms {

private:

    LaVolume*                         _source_volume;   // non-owning
    std::unique_ptr<LaVolume>         _output_volume;
    std::unique_ptr<LaVolumeGraphTraversal> _graph;

    std::vector<vtkIdType>  _seed_node_ids;

    int                  _neighbourhood_size;
    double               _sigma;
    VolumeFalloffKernel  _falloff;
    std::string          _output_array_name;

    // ------------------------------------------------------------------
    // Internal helpers
    // ------------------------------------------------------------------

    double EvaluateFalloff(double d, double max_d) const;

    static std::string UniqueArrayName(vtkUnstructuredGrid* grid,
                                       const std::string& candidate);

    double EstimateMeanEdgeLength() const;

public:

    // ------------------------------------------------------------------
    // Pipeline interface
    // ------------------------------------------------------------------

    void SetInputData(LaVolume* volume);

    /*
     * Sets volumetric node IDs directly.
     */
    void SetNodeIDList(const std::vector<vtkIdType>& ids);

    /*
     * Maps surface point IDs to the nearest volumetric nodes using
     * vtkPointLocator.  Retained as a fallback; prefer coordinate-based
     * methods for new workflows.
     */
    void SetSurfaceSeeds(LaShell* surface_shell,
                         const std::vector<int>& surface_point_ids);

    /*
     * Dispatcher. Reads a seed file in either coordinate or legacy ID format.
     *
     * Default behaviour (is_coordinates=true, guess_format=false):
     *   treats the file as coordinate format — the standard output of the
     *   interactive picker.
     *
     * guess_format=true overrides is_coordinates and auto-detects format
     *   by inspecting the first non-empty line.
     *
     * is_coordinates=false, guess_format=false: legacy integer ID format.
     *
     * Coordinates are resolved to the nearest volumetric node directly
     * via vtkPointLocator — no surface shell required.
     */
    void ReadPointsFile(const char* filename,
                        bool is_coordinates = true,
                        bool guess_format   = false);

    /*
     * Reads seed coordinates from a plain-text file (x y z per line).
     * Resolves each coordinate to the nearest volumetric node via
     * vtkPointLocator on the volumetric grid directly.
     */
    void ReadPointCoordinatesFile(const char* filename);

    /*
     * Reads volumetric node IDs from a plain-text file (one integer per line).
     * Legacy format — prefer ReadPointsFile or ReadPointCoordinatesFile.
     */
    void ReadPointIDFile(const char* filename);

    void SetNeighbourhoodSize(int n);

    /*
     * Gaussian σ in world-space units.
     * Default: auto-derived as mean_edge_length * neighbourhood_size / 2.
     */
    void SetSigma(double sigma);

    void SetFalloffToGaussian();
    void SetFalloffToLinear();

    /*
     * Output scalar array name.  Auto-suffixed (_1, _2, ...) if already
     * present on the mesh.  Default: "synthetic_scar".
     */
    void SetOutputArrayName(const char* name);

    void Update();

    LaVolume* GetOutput();

    LaVolumeSyntheticScar();
    ~LaVolumeSyntheticScar() = default;
};