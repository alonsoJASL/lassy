#define HAS_VTK 1

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include "../include/LaVolumeSyntheticScar.h"


// ============================================================
// Constructor
// ============================================================

LaVolumeSyntheticScar::LaVolumeSyntheticScar() :
    _source_volume(nullptr),
    _output_volume(std::make_unique<LaVolume>()),
    _graph(std::make_unique<LaVolumeGraphTraversal>()),
    _neighbourhood_size(15),
    _sigma(-1.0),
    _falloff(VolumeFalloffKernel::Gaussian),
    _output_array_name("synthetic_scar") {}


// ============================================================
// Public setters
// ============================================================

void LaVolumeSyntheticScar::SetInputData(LaVolume* volume) {
    _source_volume = volume;
}

void LaVolumeSyntheticScar::SetSurfaceSeeds(LaShell* surface_shell,
                                             const std::vector<int>& surface_point_ids) {
    if (!_source_volume) {
        std::cerr << "LaVolumeSyntheticScar::SetSurfaceSeeds — call SetInputData first."
             << std::endl;
        return;
    }

    // Extract surface shell poly data for the locator
    vtkSmartPointer<vtkPolyData> surface_poly =
        vtkSmartPointer<vtkPolyData>::New();
    surface_shell->GetMesh3D(surface_poly);

    // Build a point locator on the volumetric grid
    vtkSmartPointer<vtkPointLocator> locator =
        vtkSmartPointer<vtkPointLocator>::New();
    locator->SetDataSet(_source_volume->GetGridPointer());
    locator->BuildLocator();

    _seed_node_ids.clear();
    _seed_node_ids.reserve(surface_point_ids.size());

    for (const int surface_id : surface_point_ids) {
        double pt[3];
        surface_poly->GetPoint(surface_id, pt);
        const vtkIdType volume_id = locator->FindClosestPoint(pt);
        _seed_node_ids.push_back(volume_id);
    }

    std::cout << "LaVolumeSyntheticScar: mapped " << surface_point_ids.size()
         << " surface seeds to " << _seed_node_ids.size()
         << " volumetric nodes." << std::endl;
}

void LaVolumeSyntheticScar::SetNodeIDList(const std::vector<vtkIdType>& ids) {
    _seed_node_ids = ids;
}

void LaVolumeSyntheticScar::ReadPointIDFile(const char* filename) {
    _seed_node_ids.clear();
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "LaVolumeSyntheticScar::ReadPointIDFile — cannot open: "
             << filename << std::endl;
        return;
    }
    std::string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        try {
            _seed_node_ids.push_back(
                static_cast<vtkIdType>(stoi(line)));
        } catch (...) {
            std::cerr << "LaVolumeSyntheticScar::ReadPointIDFile — skipping "
                    "non-integer line: " << line << std::endl;
        }
    }
    std::cout << "Read " << _seed_node_ids.size()
         << " seed node IDs from " << filename << std::endl;
}

void LaVolumeSyntheticScar::SetNeighbourhoodSize(int n) {
    _neighbourhood_size = n;
}

void LaVolumeSyntheticScar::SetSigma(double sigma) {
    _sigma = sigma;
}

void LaVolumeSyntheticScar::SetFalloffToGaussian() {
    _falloff = VolumeFalloffKernel::Gaussian;
}

void LaVolumeSyntheticScar::SetFalloffToLinear() {
    _falloff = VolumeFalloffKernel::Linear;
}

void LaVolumeSyntheticScar::SetOutputArrayName(const char* name) {
    _output_array_name = std::string(name);
}


// ============================================================
// Private helpers
// ============================================================

double LaVolumeSyntheticScar::EvaluateFalloff(double d, double max_d) const {
    if (max_d <= 0.0) return 1.0;

    switch (_falloff) {
        case VolumeFalloffKernel::Gaussian: {
            const double effective_sigma =
                (_sigma > 0.0) ? _sigma : (max_d / 2.0);
            return exp(-(d * d) / (2.0 * effective_sigma * effective_sigma));
        }
        case VolumeFalloffKernel::Linear: {
            return std::max(0.0, 1.0 - (d / max_d));
        }
    }
    return 0.0;
}

std::string LaVolumeSyntheticScar::UniqueArrayName(vtkUnstructuredGrid* grid,
                                               const std::string& candidate) {
    const int num_arrays = grid->GetPointData()->GetNumberOfArrays();
    std::vector<std::string> existing;
    for (int i = 0; i < num_arrays; ++i) {
        const char* name = grid->GetPointData()->GetArrayName(i);
        if (name) existing.push_back(std::string(name));
    }

    auto name_taken = [&](const std::string& n) {
        return find(existing.begin(), existing.end(), n) != existing.end();
    };

    if (!name_taken(candidate)) return candidate;

    int suffix = 1;
    while (true) {
        std::string candidate_n = candidate + "_" + std::to_string(suffix);
        if (!name_taken(candidate_n)) return candidate_n;
        ++suffix;
    }
}

double LaVolumeSyntheticScar::EstimateMeanEdgeLength() const {
    vtkUnstructuredGrid* grid = _source_volume->GetGridPointer();
    const int sample_count = std::min(
        static_cast<int>(grid->GetNumberOfPoints()), 200);

    double total = 0.0;
    int sampled = 0;

    for (int i = 0; i < sample_count; ++i) {
        const auto neighbours =
            _graph->GetNeighboursAroundPoint(
                static_cast<vtkIdType>(i), 1);

        if (neighbours.size() < 2) continue;

        double pi[3], pj[3];
        grid->GetPoint(i, pi);
        // neighbours[0] is the seed itself, neighbours[1] is first hop
        grid->GetPoint(neighbours[1].first, pj);
        total += LaVolumeGraphTraversal::Euclidean(pi, pj);
        ++sampled;
    }

    return (sampled > 0) ? (total / sampled) : 1.0;
}


// ============================================================
// Update
// ============================================================

void LaVolumeSyntheticScar::Update() {
    if (!_source_volume) {
        std::cerr << "LaVolumeSyntheticScar::Update — no input set." << std::endl;
        return;
    }
    if (_seed_node_ids.size() < 2) {
        std::cerr << "LaVolumeSyntheticScar::Update — at least 2 seed node IDs "
                "required." << std::endl;
        return;
    }

    vtkUnstructuredGrid* grid = _source_volume->GetGridPointer();
    const vtkIdType num_points = grid->GetNumberOfPoints();

    if (num_points == 0) {
        std::cerr << "LaVolumeSyntheticScar::Update — source grid has no points."
             << std::endl;
        return;
    }

    // ---- Step 1: build graph ------------------------------------------
    _graph->SetInputGrid(grid);
    _graph->SetEdgeWeightToEuclidean();
    _graph->Build();

    // ---- Step 2: build path vertices ----------------------------------
    std::map<vtkIdType, int> path_vertex_map;
    const int num_seeds = static_cast<int>(_seed_node_ids.size());

    for (int i = 0; i < num_seeds; ++i) {
        const vtkIdType start = _seed_node_ids[static_cast<size_t>(i)];
        const vtkIdType end   =
            _seed_node_ids[static_cast<size_t>((i + 1) % num_seeds)];

        const std::vector<vtkIdType> path = _graph->ShortestPath(start, end);
        for (const vtkIdType v : path) {
            path_vertex_map.insert({v, 1});
        }
    }

    std::vector<vtkIdType> path_vertices;
    path_vertices.reserve(path_vertex_map.size());
    for (const auto& kv : path_vertex_map) {
        path_vertices.push_back(kv.first);
    }

    std::cout << "Path built from " << num_seeds << " seeds, "
         << path_vertices.size() << " unique path nodes." << std::endl;

    // ---- Step 3: estimate world-space corridor radius -----------------
    const double mean_edge = EstimateMeanEdgeLength();
    const double max_d     = mean_edge * _neighbourhood_size;

    std::cout << "Estimated mean edge length: " << mean_edge
         << ", corridor radius (max_d): " << max_d << std::endl;

    // ---- Step 4: accumulate scalar contributions ----------------------
    std::vector<double> accumulator(static_cast<size_t>(num_points), 0.0);

    for (const vtkIdType path_vtx : path_vertices) {
        double path_point[3];
        grid->GetPoint(path_vtx, path_point);

        // Path vertex itself — distance 0
        accumulator[static_cast<size_t>(path_vtx)] +=
            EvaluateFalloff(0.0, max_d);

        // N-hop neighbourhood
        const auto neighbours =
            _graph->GetNeighboursAroundPoint(path_vtx, _neighbourhood_size);

        for (const auto& [neighbour_id, depth] : neighbours) {
            if (neighbour_id < 0 ||
                neighbour_id >= static_cast<vtkIdType>(num_points)) continue;

            double neighbour_point[3];
            grid->GetPoint(neighbour_id, neighbour_point);
            const double dist =
                LaVolumeGraphTraversal::Euclidean(path_point, neighbour_point);

            accumulator[static_cast<size_t>(neighbour_id)] +=
                EvaluateFalloff(dist, max_d);
        }
    }

    // ---- Step 5: normalise to [0, 1] ----------------------------------
    const double max_acc =
        *max_element(accumulator.begin(), accumulator.end());

    if (max_acc <= 0.0) {
        std::cerr << "LaVolumeSyntheticScar::Update — all accumulator values are "
                "zero. Check seed IDs and neighbourhood size." << std::endl;
        return;
    }

    // ---- Step 6: write scalar array -----------------------------------
    vtkSmartPointer<vtkUnstructuredGrid> output_grid =
        vtkSmartPointer<vtkUnstructuredGrid>::New();
    output_grid->DeepCopy(grid);

    const std::string array_name = UniqueArrayName(output_grid, _output_array_name);
    std::cout << "Writing scalar array: " << array_name << std::endl;

    vtkSmartPointer<vtkFloatArray> scar_scalars =
        vtkSmartPointer<vtkFloatArray>::New();
    scar_scalars->SetName(array_name.c_str());
    scar_scalars->SetNumberOfComponents(1);

    for (vtkIdType i = 0; i < num_points; ++i) {
        scar_scalars->InsertNextValue(
            static_cast<float>(
                accumulator[static_cast<size_t>(i)] / max_acc));
    }

    output_grid->GetPointData()->AddArray(scar_scalars);
    _output_volume->SetGrid(output_grid);

    std::cout << "LaVolumeSyntheticScar::Update complete. Array \""
         << array_name << "\" added to output volume." << std::endl;
}


// ============================================================
// GetOutput
// ============================================================

LaVolume* LaVolumeSyntheticScar::GetOutput() {
    return _output_volume.get();
}