#define HAS_VTK 1

#include <iostream>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#include <vtkCell.h>
#include <vtkIdList.h>

#include "../include/LaVolumeGraphTraversal.h"


// ============================================================
// Constructor
// ============================================================

LaVolumeGraphTraversal::LaVolumeGraphTraversal() :
    _grid(nullptr),
    _weight_mode(EdgeWeight::Euclidean),
    _built(false),
    _num_nodes(0) {}


// ============================================================
// Setup
// ============================================================

void LaVolumeGraphTraversal::SetInputGrid(vtkUnstructuredGrid* grid) {
    _grid  = grid;
    _built = false;
    _adj.clear();
    _num_nodes = 0;
}

void LaVolumeGraphTraversal::SetEdgeWeightToEuclidean() {
    _weight_mode = EdgeWeight::Euclidean;
}

void LaVolumeGraphTraversal::SetEdgeWeightToTopological() {
    _weight_mode = EdgeWeight::Topological;
}

void LaVolumeGraphTraversal::InsertCellEdges(vtkIdList* point_ids) {
    const vtkIdType n = point_ids->GetNumberOfIds();
    for (vtkIdType i = 0; i < n; ++i) {
        for (vtkIdType j = i + 1; j < n; ++j) {
            const vtkIdType a = point_ids->GetId(i);
            const vtkIdType b = point_ids->GetId(j);

            double weight = 1.0;
            if (_weight_mode == EdgeWeight::Euclidean) {
                double pa[3], pb[3];
                _grid->GetPoint(a, pa);
                _grid->GetPoint(b, pb);
                weight = Euclidean(pa, pb);
            }

            // Insert both directions — undirected graph
            _adj[static_cast<size_t>(a)].push_back({b, weight});
            _adj[static_cast<size_t>(b)].push_back({a, weight});
        }
    }
}

void LaVolumeGraphTraversal::Build() {
    if (!_grid) {
        std::cerr << "LaVolumeGraphTraversal::Build — no grid set. "
                "Call SetInputGrid() first." << std::endl;
        return;
    }

    _num_nodes = _grid->GetNumberOfPoints();
    _adj.assign(static_cast<size_t>(_num_nodes), {});

    const vtkIdType num_cells = _grid->GetNumberOfCells();
    for (vtkIdType c = 0; c < num_cells; ++c) {
        vtkSmartPointer<vtkIdList> point_ids =
            vtkSmartPointer<vtkIdList>::New();
        _grid->GetCellPoints(c, point_ids);
        InsertCellEdges(point_ids);
    }

    // Deduplicate edges per node (multiple cells may share the same edge)
    for (auto& neighbours : _adj) {
        sort(neighbours.begin(), neighbours.end());
        neighbours.erase(unique(neighbours.begin(), neighbours.end()),
                         neighbours.end());
    }

    _built = true;
    std::cout << "LaVolumeGraphTraversal::Build complete — "
         << _num_nodes << " nodes, " << num_cells << " cells." << std::endl;
}


// ============================================================
// ShortestPath — Dijkstra
// ============================================================

std::vector<vtkIdType> LaVolumeGraphTraversal::ShortestPath(vtkIdType start_id,
                                                        vtkIdType end_id) const {
    if (!_built) {
        std::cerr << "LaVolumeGraphTraversal::ShortestPath — graph not built. "
                "Call Build() first." << std::endl;
        return {};
    }

    const size_t n = static_cast<size_t>(_num_nodes);
    constexpr double inf = std::numeric_limits<double>::infinity();

    std::vector<double>    dist(n, inf);
    std::vector<vtkIdType> predecessor(n, -1);

    // Min-heap: (distance, node_id)
    std::priority_queue<std::pair<double, vtkIdType>,
                   std::vector<std::pair<double, vtkIdType>>,
                   std::greater<std::pair<double, vtkIdType>>> pq;

    dist[static_cast<size_t>(start_id)] = 0.0;
    pq.push({0.0, start_id});

    while (!pq.empty()) {
        const auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[static_cast<size_t>(u)]) continue; // stale entry
        if (u == end_id) break;                          // early exit

        for (const auto& [v, w] : _adj[static_cast<size_t>(u)]) {
            const double new_dist = dist[static_cast<size_t>(u)] + w;
            if (new_dist < dist[static_cast<size_t>(v)]) {
                dist[static_cast<size_t>(v)] = new_dist;
                predecessor[static_cast<size_t>(v)] = u;
                pq.push({new_dist, v});
            }
        }
    }

    // Reconstruct path
    std::vector<vtkIdType> path;
    if (dist[static_cast<size_t>(end_id)] == inf) {
        std::cerr << "LaVolumeGraphTraversal::ShortestPath — no path from "
             << start_id << " to " << end_id << std::endl;
        return {};
    }

    for (vtkIdType cur = end_id; cur != -1;
        cur = predecessor[static_cast<size_t>(cur)]) {
        path.push_back(cur);
    }
    std::reverse(path.begin(), path.end());
    return path;
}


// ============================================================
// GetNeighboursAroundPoint — BFS
// ============================================================

std::vector<std::pair<vtkIdType, int>> LaVolumeGraphTraversal::GetNeighboursAroundPoint(
    vtkIdType point_id,
    int       max_hops) const {

    if (!_built) {
        std::cerr << "LaVolumeGraphTraversal::GetNeighboursAroundPoint — "
                "graph not built." << std::endl;
        return {};
    }

    std::vector<std::pair<vtkIdType, int>> result;
    std::unordered_map<vtkIdType, int> visited;

    // BFS queue: (node_id, depth)
    std::queue<std::pair<vtkIdType, int>> bfs;
    bfs.push({point_id, 0});
    visited[point_id] = 0;

    while (!bfs.empty()) {
        const auto [node, depth] = bfs.front();
        bfs.pop();

        result.push_back({node, depth});

        if (depth >= max_hops) continue;

        for (const auto& [neighbour, weight] : _adj[static_cast<size_t>(node)]) {
            if (visited.find(neighbour) == visited.end()) {
                visited[neighbour] = depth + 1;
                bfs.push({neighbour, depth + 1});
            }
        }
    }

    return result;
}


// ============================================================
// Metadata + static utilities
// ============================================================

vtkIdType LaVolumeGraphTraversal::GetNumberOfNodes() const {
    return _num_nodes;
}

double LaVolumeGraphTraversal::Euclidean(const double* a, const double* b) {
    const double dx = a[0] - b[0];
    const double dy = a[1] - b[1];
    const double dz = a[2] - b[2];
    return sqrt(dx*dx + dy*dy + dz*dz);
}