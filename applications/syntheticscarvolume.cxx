#define HAS_VTK 1

#include "LaVolume.h"
#include "LaVolumeSyntheticScar.h"
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
 *  Generates a synthetic scar scalar field on a volumetric mesh (LaVolume).
 *
 *  Seed points can be provided in two ways:
 *
 *    -pts <file>   Non-interactive. Reads seed coordinates from a text file.
 *                  Format: one point per line, "x y z" (space-separated).
 *                  Coordinates are resolved to the nearest volumetric node
 *                  directly via vtkPointLocator — no surface mapping needed.
 *                  Legacy single-integer-per-line format also accepted.
 *
 *    --pick        Interactive. Extracts the surface from the volume, opens
 *                  the VTK viewer (press 'x' to pick, 's' to confirm).
 *                  Saves coordinates to <o>_seeds.txt for reproducibility,
 *                  then resolves them to volumetric nodes in memory.
 */

static void SaveCoordinates(const char* path,
                             const std::vector<std::array<double, 3>>& pts) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "syntheticScarVolume: cannot write seed file: " << path
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
            "Generates a synthetic scar scalar field on a volumetric mesh.\n"
            "\nUsage:\n"
            "  syntheticScarVolume -i <mesh.vtk|vtu> -pts <coords.txt> -o <out.vtk> [options]\n"
            "  syntheticScarVolume -i <mesh.vtk|vtu> --pick             -o <out.vtk> [options]\n"
            "\n(Mandatory)\n"
            "  -i   <mesh>       Input volumetric mesh (.vtk or .vtu)\n"
            "  -o   <out>        Output mesh (.vtk or .vtu)\n"
            "  -pts <coords.txt> Seed coordinate file (x y z per line)\n"
            "                    OR use --pick for interactive picking on extracted surface\n"
            "\n(Optional)\n"
            "  -n        <int>   Neighbourhood hops (default: 15)\n"
            "  -sigma    <float> Gaussian sigma in world units (default: auto)\n"
            "  -falloff  <int>   1=Gaussian (default), 2=Linear\n"
            "  -name     <str>   Output array name (default: synthetic_scar)\n"
            "  --pick            Interactive picking on extracted surface\n"
            << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Load volume
    // ----------------------------------------------------------------
    LaVolume* volume = new LaVolume(input_fn);

    // ----------------------------------------------------------------
    // Build algorithm
    // ----------------------------------------------------------------
    LaVolumeSyntheticScar* algorithm = new LaVolumeSyntheticScar();
    algorithm->SetInputData(volume);
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
        std::cout << "\nInteractive mode (picking on extracted surface):\n"
                     "  Press 'x' to pick seed points.\n"
                     "  Press 's' to confirm and close the viewer.\n\n";

        LaShell* surface_shell = volume->ConvertToSurface();

        LaShellGapsInBinary* picker = new LaShellGapsInBinary();
        picker->SetInputData(surface_shell);
        picker->Run();

        const auto& positions = picker->GetPickedPositions();

        if (positions.empty()) {
            std::cerr << "syntheticScarVolume: no points picked. Exiting."
                      << std::endl;
            return 1;
        }

        // Save coordinates for reproducibility
        std::string seed_file = std::string(output_fn) + "_seeds.txt";
        SaveCoordinates(seed_file.c_str(), positions);

        // Resolve directly to volumetric nodes in memory.
        // Coordinates are in the same physical space — no surface mapping.
        vtkSmartPointer<vtkPointLocator> locator =
            vtkSmartPointer<vtkPointLocator>::New();
        locator->SetDataSet(volume->GetGridPointer());
        locator->BuildLocator();

        std::vector<vtkIdType> seed_ids;
        seed_ids.reserve(positions.size());
        for (const auto& p : positions) {
            double pt[3] = {p[0], p[1], p[2]};
            seed_ids.push_back(locator->FindClosestPoint(pt));
        }
        algorithm->SetNodeIDList(seed_ids);

        delete surface_shell;
        // picker intentionally not deleted — RAII pass
    } else {
        // Default: treat as coordinate file (is_coordinates=true)
        algorithm->ReadPointsFile(pts_fn);
    }

    // ----------------------------------------------------------------
    // Run
    // ----------------------------------------------------------------
    algorithm->Update();

    LaVolume* output = algorithm->GetOutput();
    const std::string out_str(output_fn);
    if (out_str.size() >= 4 &&
        out_str.substr(out_str.size() - 4) == ".vtu") {
        output->ExportVTU(output_fn);
    } else {
        output->ExportVTK(output_fn);
    }

    delete algorithm;
    delete volume;

    return 0;
}