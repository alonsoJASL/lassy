#define HAS_VTK 1

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

#include "../include/LaShellSyntheticScar.h"

using namespace std;


// ============================================================
// Constructor / Destructor
// ============================================================

LaShellSyntheticScar::LaShellSyntheticScar() {
    _source_la         = new LaShell();
    _output_la         = new LaShell();
    _source_poly       = vtkSmartPointer<vtkPolyData>::New();
    _neighbourhood_size = 3;
    _sigma             = -1.0;   // sentinel: recompute from neighbourhood_size
    _falloff           = FalloffKernel::Gaussian;
    _output_array_name = "synthetic_scar";
}

LaShellSyntheticScar::~LaShellSyntheticScar() {
    delete _source_la;
    delete _output_la;
}


// ============================================================
// Public setters
// ============================================================

void LaShellSyntheticScar::SetInputData(LaShell* shell) {
    _source_la = shell;
    _source_la->GetMesh3D(_source_poly);
}

void LaShellSyntheticScar::SetPointIDList(const std::vector<int>& ids) {
    _seed_point_ids = ids;
}

void LaShellSyntheticScar::ReadPointIDFile(const char* filename) {
    _seed_point_ids.clear();
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "LaShellSyntheticScar::ReadPointIDFile — cannot open: "
             << filename << endl;
        return;
    }
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        try {
            _seed_point_ids.push_back(stoi(line));
        } catch (...) {
            cerr << "LaShellSyntheticScar::ReadPointIDFile — skipping "
                    "non-integer line: " << line << endl;
        }
    }
    cout << "Read " << _seed_point_ids.size()
         << " seed point IDs from " << filename << endl;
}

void LaShellSyntheticScar::SetNeighbourhoodSize(int n) {
    _neighbourhood_size = n;
}

void LaShellSyntheticScar::SetSigma(double sigma) {
    _sigma = sigma;
}

void LaShellSyntheticScar::SetFalloffToGaussian() {
    _falloff = FalloffKernel::Gaussian;
}

void LaShellSyntheticScar::SetFalloffToLinear() {
    _falloff = FalloffKernel::Linear;
}

void LaShellSyntheticScar::SetOutputArrayName(const char* name) {
    _output_array_name = string(name);
}


// ============================================================
// Private helpers
// ============================================================

void LaShellSyntheticScar::GetConnectedVertices(
    vtkSmartPointer<vtkPolyData> mesh,
    int seed,
    vtkSmartPointer<vtkIdList> connected_vertices) {

    vtkSmartPointer<vtkIdList> cell_id_list = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPointCells(seed, cell_id_list);

    for (vtkIdType i = 0; i < cell_id_list->GetNumberOfIds(); ++i) {
        vtkCell* cell = mesh->GetCell(cell_id_list->GetId(i));
        if (cell->GetNumberOfEdges() <= 0) continue;

        for (vtkIdType e = 0; e < cell->GetNumberOfEdges(); ++e) {
            vtkCell* edge = cell->GetEdge(e);
            vtkIdList* point_id_list = edge->GetPointIds();

            if (point_id_list->GetId(0) == seed) {
                connected_vertices->InsertNextId(point_id_list->GetId(1));
            } else if (point_id_list->GetId(1) == seed) {
                connected_vertices->InsertNextId(point_id_list->GetId(0));
            }
        }
    }
}

void LaShellSyntheticScar::RecursivePointNeighbours(vtkIdType point_id,
                                                     int order) {
    if (order == 0) return;

    // Check if already visited
    for (const auto& visited : _visited_point_list) {
        if (visited.first == point_id) return;
    }
    _visited_point_list.push_back({point_id, order});

    vtkSmartPointer<vtkIdList> point_list = vtkSmartPointer<vtkIdList>::New();
    GetConnectedVertices(_source_poly, static_cast<int>(point_id), point_list);

    for (vtkIdType e = 0; e < point_list->GetNumberOfIds(); ++e) {
        RecursivePointNeighbours(point_list->GetId(e), order - 1);
    }
}

void LaShellSyntheticScar::GetNeighboursAroundPoint(
    int point_id,
    std::vector<std::pair<int, int>>& neighbours) {

    _visited_point_list.clear();
    RecursivePointNeighbours(static_cast<vtkIdType>(point_id),
                             _neighbourhood_size);

    for (const auto& v : _visited_point_list) {
        neighbours.push_back({static_cast<int>(v.first), v.second});
    }
}

std::vector<vtkIdType> LaShellSyntheticScar::BuildPathVertices() {
    // Use a map for O(log n) deduplication while preserving determinism
    map<vtkIdType, int> vertex_ids;
    const int num_seeds = static_cast<int>(_seed_point_ids.size());

    for (int i = 0; i < num_seeds; ++i) {
        int start_id = _seed_point_ids[i];
        int end_id   = _seed_point_ids[(i + 1) % num_seeds]; // closes loop

        vtkSmartPointer<vtkDijkstraGraphGeodesicPath> dijkstra =
            vtkSmartPointer<vtkDijkstraGraphGeodesicPath>::New();
        dijkstra->SetInputData(_source_poly);
        dijkstra->UseScalarWeightsOff();   // topology-only weighting
        dijkstra->SetStartVertex(start_id);
        dijkstra->SetEndVertex(end_id);
        dijkstra->Update();

        vtkIdList* path_vertices = dijkstra->GetIdList();
        for (vtkIdType j = 0; j < path_vertices->GetNumberOfIds(); ++j) {
            vertex_ids.insert({path_vertices->GetId(j), 1});
        }
    }

    std::vector<vtkIdType> result;
    result.reserve(vertex_ids.size());
    for (const auto& kv : vertex_ids) {
        result.push_back(kv.first);
    }
    cout << "Path built from " << num_seeds << " seeds, "
         << result.size() << " unique path vertices." << endl;
    return result;
}

double LaShellSyntheticScar::Euclidean(const double* a, const double* b) {
    const double dx = a[0] - b[0];
    const double dy = a[1] - b[1];
    const double dz = a[2] - b[2];
    return sqrt(dx*dx + dy*dy + dz*dz);
}

double LaShellSyntheticScar::EvaluateFalloff(double d, double max_d) const {
    if (max_d <= 0.0) return 1.0;

    switch (_falloff) {
        case FalloffKernel::Gaussian: {
            // Use effective_sigma: either user-set or derived from max_d
            // so that at d == max_d the weight is exp(-0.5 * (max_d/sigma)^2)
            // With sigma = max_d / 2, that gives exp(-2) ≈ 0.135 at the edge —
            // a smooth, non-zero tail that avoids a hard cutoff.
            const double effective_sigma = (_sigma > 0.0) ? _sigma : (max_d / 2.0);
            return exp(-(d * d) / (2.0 * effective_sigma * effective_sigma));
        }
        case FalloffKernel::Linear: {
            return max(0.0, 1.0 - (d / max_d));
        }
    }
    return 0.0;  // unreachable, satisfies compiler
}

std::string LaShellSyntheticScar::UniqueArrayName(vtkPolyData* poly,
                                                   const std::string& candidate) {
    const int num_arrays = poly->GetPointData()->GetNumberOfArrays();

    // Build a set of existing names for O(1) lookup
    vector<string> existing;
    for (int i = 0; i < num_arrays; ++i) {
        const char* name = poly->GetPointData()->GetArrayName(i);
        if (name) existing.push_back(string(name));
    }

    auto name_taken = [&](const string& n) {
        return find(existing.begin(), existing.end(), n) != existing.end();
    };

    if (!name_taken(candidate)) return candidate;

    int suffix = 1;
    while (true) {
        string candidate_n = candidate + "_" + to_string(suffix);
        if (!name_taken(candidate_n)) return candidate_n;
        ++suffix;
    }
}


// ============================================================
// Update — main pipeline
// ============================================================

void LaShellSyntheticScar::Update() {
    if (_seed_point_ids.size() < 2) {
        cerr << "LaShellSyntheticScar::Update — at least 2 seed point IDs "
                "required. Aborting." << endl;
        return;
    }

    const vtkIdType num_points = _source_poly->GetNumberOfPoints();
    if (num_points == 0) {
        cerr << "LaShellSyntheticScar::Update — source mesh has no points. "
                "Was SetInputData called?" << endl;
        return;
    }

    // ---- Step 1: build path vertices ----------------------------------
    const std::vector<vtkIdType> path_vertices = BuildPathVertices();

    // ---- Step 2: accumulate scalar contributions ----------------------
    // accumulator[i] holds the sum of falloff contributions at vertex i.
    // Initialised to 0.0; left at 0 for vertices outside the corridor.
    std::vector<double> accumulator(static_cast<size_t>(num_points), 0.0);

    // Estimate max_d as the mean edge length × neighbourhood_size.
    // This gives a world-space radius consistent with the hop count.
    // Computed once from a sample of edges to avoid a full traversal.
    double sample_edge_length = 0.0;
    {
        const int sample_count = min(static_cast<int>(num_points), 200);
        int sampled = 0;
        for (int i = 0; i < sample_count; ++i) {
            vtkSmartPointer<vtkIdList> neighbours =
                vtkSmartPointer<vtkIdList>::New();
            GetConnectedVertices(_source_poly, i, neighbours);
            if (neighbours->GetNumberOfIds() == 0) continue;
            double p[3], q[3];
            _source_poly->GetPoint(i, p);
            _source_poly->GetPoint(neighbours->GetId(0), q);
            sample_edge_length += Euclidean(p, q);
            ++sampled;
        }
        if (sampled > 0) sample_edge_length /= sampled;
        else             sample_edge_length  = 1.0;  // fallback, avoid /0
    }
    const double max_d = sample_edge_length * _neighbourhood_size;

    cout << "Estimated mean edge length: " << sample_edge_length
         << ", corridor radius (max_d): " << max_d << endl;

    // For each path vertex, expand its neighbourhood and accumulate
    for (const vtkIdType path_vtx : path_vertices) {
        double path_point[3];
        _source_poly->GetPoint(path_vtx, path_point);

        // Accumulate the path vertex itself
        accumulator[static_cast<size_t>(path_vtx)] +=
            EvaluateFalloff(0.0, max_d);

        // Expand N-order neighbourhood
        std::vector<std::pair<int, int>> neighbours;
        GetNeighboursAroundPoint(static_cast<int>(path_vtx), neighbours);

        for (const auto& [neighbour_id, depth] : neighbours) {
            if (neighbour_id < 0 ||
                neighbour_id >= static_cast<int>(num_points)) continue;

            double neighbour_point[3];
            _source_poly->GetPoint(neighbour_id, neighbour_point);
            const double dist = Euclidean(path_point, neighbour_point);

            accumulator[static_cast<size_t>(neighbour_id)] +=
                EvaluateFalloff(dist, max_d);
        }
    }

    // ---- Step 3: normalise to [0, 1] ----------------------------------
    const double max_acc =
        *max_element(accumulator.begin(), accumulator.end());

    if (max_acc <= 0.0) {
        cerr << "LaShellSyntheticScar::Update — all accumulator values are "
                "zero. Check seed IDs and neighbourhood size." << endl;
        return;
    }

    // ---- Step 4: write scalar array to a copy of the source mesh ------
    vtkSmartPointer<vtkPolyData> output_poly =
        vtkSmartPointer<vtkPolyData>::New();
    output_poly->DeepCopy(_source_poly);

    const std::string array_name =
        UniqueArrayName(output_poly, _output_array_name);
    cout << "Writing scalar array: " << array_name << endl;

    vtkSmartPointer<vtkFloatArray> scar_scalars =
        vtkSmartPointer<vtkFloatArray>::New();
    scar_scalars->SetName(array_name.c_str());
    scar_scalars->SetNumberOfComponents(1);

    for (vtkIdType i = 0; i < num_points; ++i) {
        const float normalised =
            static_cast<float>(accumulator[static_cast<size_t>(i)] / max_acc);
        scar_scalars->InsertNextValue(normalised);
    }

    output_poly->GetPointData()->AddArray(scar_scalars);
    _output_la->SetMesh3D(output_poly);

    cout << "LaShellSyntheticScar::Update complete. "
         << "Array \"" << array_name << "\" added to output mesh." << endl;
}


// ============================================================
// GetOutput
// ============================================================

LaShell* LaShellSyntheticScar::GetOutput() {
    return _output_la;
}