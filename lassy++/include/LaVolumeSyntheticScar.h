/*
 *  LaVolumeSyntheticScar.h
 *
 *  Generates a synthetic scar scalar field on a volumetric mesh (LaVolume).
 *
 *  Mirrors the public API of LaShellSyntheticScar exactly, with one
 *  addition: SetSurfaceSeeds(), which accepts a surface shell and surface
 *  point IDs (e.g. picked interactively via the encirclement viewer) and
 *  maps them to the nearest volumetric nodes via vtkPointLocator.
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

    LaVolume*  _source_volume;              // non-owning
    std::unique_ptr<LaVolume>  _output_volume;

    std::unique_ptr<LaVolumeGraphTraversal> _graph;

    std::vector<vtkIdType>  _seed_node_ids; // volumetric node IDs

    int                  _neighbourhood_size;
    double               _sigma;            // -1 = auto-derive
    VolumeFalloffKernel  _falloff;
    std::string          _output_array_name;

    // ------------------------------------------------------------------
    // Internal helpers
    // ------------------------------------------------------------------

    /*
     * Evaluates the chosen falloff kernel.
     * d     — distance from corridor point to nearest path node
     * max_d — normalisation radius in world units
     */
    double EvaluateFalloff(double d, double max_d) const;

    /*
     * Scans existing point data arrays on the grid for name collisions
     * with candidate.  Appends _1, _2, ... until the name is unique.
     */
    static std::string UniqueArrayName(vtkUnstructuredGrid* grid,
                                       const std::string& candidate);

    /*
     * Estimates mean edge length from a sample of nodes.
     * Used to derive max_d (world-space corridor radius).
     */
    double EstimateMeanEdgeLength() const;

public:

    // ------------------------------------------------------------------
    // Pipeline interface
    // ------------------------------------------------------------------

    void SetInputData(LaVolume* volume);

    /*
     * Maps surface point IDs to the nearest volumetric nodes using
     * vtkPointLocator.  Call this when seeds were picked on a surface
     * shell derived from this volume.
     */
    void SetSurfaceSeeds(LaShell* surface_shell,
                         const std::vector<int>& surface_point_ids);

    /*
     * Sets volumetric node IDs directly — use when you already have
     * IDs in the volume's coordinate space.
     */
    void SetNodeIDList(const std::vector<vtkIdType>& ids);

    /*
     * Reads seed IDs from a plain-text file (one integer per line).
     * IDs are interpreted as volumetric node IDs unless
     * SetSurfaceSeeds() is used instead.
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