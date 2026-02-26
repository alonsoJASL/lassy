#define HAS_VTK 1

#include "LaShell.h"
#include "LaShellSyntheticScar.h"
#include "LaShellGapsInBinary.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <vector>

/*
 *  Author:
 *  Dr. José Alonso Solís-Lemus
 *
 *  Generates a synthetic scar scalar field on a surface mesh (LaShell).
 *
 *  Seed points can be provided in two ways:
 *
 *    -pts <file>   Non-interactive. Reads seed coordinates from a text file.
 *                  Format: one point per line, "x y z" (space-separated).
 *                  Legacy single-integer-per-line format also accepted.
 *
 *    --pick        Interactive. Opens the VTK viewer (press 'x' to pick,
 *                  's' to confirm). Saves coordinates to <o>_seeds.txt
 *                  for reproducibility, then runs scar generation in
 *                  memory without a file read-back.
 */

static void SaveCoordinates(const char* path,
                             const std::vector<std::array<double, 3>>& pts) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "syntheticScar: cannot write seed file: " << path
                  << std::endl;
        return;
    }
    for (const auto& p : pts)
        out << p[0] << " " << p[1] << " " << p[2] << "\n";
    std::cout << "Seed coordinates saved: " << path << std::endl;
}

int main(int argc, char* argv[]) {
    const char* input_fn   = nullptr;
    const char* pts_fn     = nullptr;
    const char* output_fn  = nullptr;
    const char* array_name = nullptr;
    int    neighbourhood   = 15;
    double sigma           = -1.0;
    int    falloff_mode    = 1;
    bool   interactive     = false;

    bool found_input  = false;
    bool found_output = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--pick") { interactive = true; continue; }
        if (i + 1 == argc)  continue;
        if      (arg == "-i")       { input_fn      = argv[++i]; found_input  = true; }
        else if (arg == "-pts")     { pts_fn        = argv[++i]; }
        else if (arg == "-o")       { output_fn     = argv[++i]; found_output = true; }
        else if (arg == "-n")       { neighbourhood = atoi(argv[++i]); }
        else if (arg == "-sigma")   { sigma         = atof(argv[++i]); }
        else if (arg == "-falloff") { falloff_mode  = atoi(argv[++i]); }
        else if (arg == "-name")    { array_name    = argv[++i]; }
    }

    const bool has_pts = (pts_fn != nullptr);

    if (!found_input || !found_output || (!has_pts && !interactive)) {
        std::cerr <<
            "Generates a synthetic scar scalar field on a surface mesh.\n"
            "\nUsage:\n"
            "  syntheticScar -i <mesh.vtk> -pts <coords.txt> -o <out.vtk> [options]\n"
            "  syntheticScar -i <mesh.vtk> --pick             -o <out.vtk> [options]\n"
            "\n(Mandatory)\n"
            "  -i   <mesh>       Input surface mesh (.vtk)\n"
            "  -o   <out>        Output mesh (.vtk)\n"
            "  -pts <coords.txt> Seed coordinate file (x y z per line)\n"
            "                    OR use --pick for interactive picking\n"
            "\n(Optional)\n"
            "  -n        <int>   Neighbourhood hops (default: 15)\n"
            "  -sigma    <float> Gaussian sigma in world units (default: auto)\n"
            "  -falloff  <int>   1=Gaussian (default), 2=Linear\n"
            "  -name     <str>   Output array name (default: synthetic_scar)\n"
            "  --pick            Interactive picking\n"
            << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Load mesh
    // ----------------------------------------------------------------
    LaShell* source_mesh = new LaShell(input_fn);

    // ----------------------------------------------------------------
    // Build algorithm
    // ----------------------------------------------------------------
    LaShellSyntheticScar* algorithm = new LaShellSyntheticScar();
    algorithm->SetInputData(source_mesh);
    algorithm->SetNeighbourhoodSize(neighbourhood);
    if (sigma > 0.0) algorithm->SetSigma(sigma);
    if (falloff_mode == 2) {
        std::cout << "Falloff: Linear\n";
        algorithm->SetFalloffToLinear();
    } else {
        std::cout << "Falloff: Gaussian\n";
        algorithm->SetFalloffToGaussian();
    }
    if (array_name != nullptr) algorithm->SetOutputArrayName(array_name);

    // ----------------------------------------------------------------
    // Seeds
    // ----------------------------------------------------------------
    if (interactive) {
        std::cout << "\nInteractive mode:\n"
                     "  Press 'x' to pick seed points on the mesh.\n"
                     "  Press 's' to confirm and close the viewer.\n\n";

        LaShellGapsInBinary* picker = new LaShellGapsInBinary();
        picker->SetInputData(source_mesh);
        picker->Run();

        const auto& positions = picker->GetPickedPositions();

        if (positions.empty()) {
            std::cerr << "syntheticScar: no points picked. Exiting." << std::endl;
            return 1;
        }

        // Save coordinates for reproducibility
        std::string seed_file = std::string(output_fn) + "_seeds.txt";
        SaveCoordinates(seed_file.c_str(), positions);

        // Resolve to mesh point IDs in memory — no file read-back needed
        vtkSmartPointer<vtkPolyData> source_poly =
            vtkSmartPointer<vtkPolyData>::New();
        source_mesh->GetMesh3D(source_poly);

        vtkSmartPointer<vtkPointLocator> locator =
            vtkSmartPointer<vtkPointLocator>::New();
        locator->SetDataSet(source_poly);
        locator->BuildLocator();

        std::vector<int> seed_ids;
        seed_ids.reserve(positions.size());
        for (const auto& p : positions) {
            double pt[3] = {p[0], p[1], p[2]};
            seed_ids.push_back(
                static_cast<int>(locator->FindClosestPoint(pt)));
        }
        algorithm->SetPointIDList(seed_ids);

        // picker intentionally not deleted — RAII pass
    } else {
        // Default: treat as coordinate file (is_coordinates=true)
        algorithm->ReadPointsFile(pts_fn);
    }

    // ----------------------------------------------------------------
    // Run
    // ----------------------------------------------------------------
    algorithm->Update();

    LaShell* output = algorithm->GetOutput();
    output->ExportVTK(const_cast<char*>(output_fn));
    std::cout << "Saved: " << output_fn << std::endl;

    delete algorithm;
    delete source_mesh;

    return 0;
}