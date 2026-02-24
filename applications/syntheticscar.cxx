#define HAS_VTK 1

#include "LaShellSyntheticScar.h"
#include "LaShellGapsInBinary.h"   // for interactive picking via Run()

#include <iostream>
#include <string>
#include <vector>

#define DEFAULT_NEIGHBOURHOOD_SIZE 15

/*
 *  Author:
 *  Dr. José Alonso Solís-Lemus
 *
 *  Generates a synthetic scar scalar field on a vtkPolyData mesh.
 *
 *  Seed point IDs define a closed boundary path on the surface.
 *  Dijkstra shortest paths connect consecutive seeds (closing the loop).
 *  Each mesh vertex within the neighbourhood radius receives a
 *  distance-weighted scalar contribution; contributions are summed so
 *  converging path segments produce higher intensity naturally.
 *  The result is normalised to [0, 1] and written as a named scalar array.
 *
 *  Two modes:
 *    --file     Non-interactive.  Reads seed IDs from a text file
 *               (one integer per line — same format written by encirclement).
 *    --pick     Interactive.  Opens the VTK viewer so the user can pick
 *               points on the mesh (press 'x'), then press 's' to save IDs
 *               and exit; the application then runs the scar generation.
 */
int main(int argc, char* argv[]) {
    const char* input_fn    = nullptr;
    const char* pts_fn      = nullptr;
    const char* output_fn   = nullptr;
    const char* array_name  = nullptr;
    int    neighbourhood    = DEFAULT_NEIGHBOURHOOD_SIZE;
    double sigma            = -1.0;   // sentinel: auto-derive
    int    falloff_mode     = 1;      // 1 = Gaussian (default), 2 = Linear
    bool   interactive_mode = false;

    bool found_input  = false;
    bool found_output = false;

    if (argc < 2) {
        std::cerr << "syntheticScar: too few arguments.\n";
    } else {
        for (int i = 1; i < argc; ++i) {
            const std::string arg(argv[i]);

            if (arg == "--pick") {
                interactive_mode = true;
                continue;
            }

            if (i + 1 == argc) continue;   // flag needs a value; skip

            if (arg == "-i") {
                input_fn   = argv[++i];
                found_input = true;
            } else if (arg == "-pts") {
                pts_fn = argv[++i];
            } else if (arg == "-o") {
                output_fn   = argv[++i];
                found_output = true;
            } else if (arg == "-n") {
                neighbourhood = std::atoi(argv[++i]);
            } else if (arg == "-sigma") {
                sigma = std::atof(argv[++i]);
            } else if (arg == "-falloff") {
                falloff_mode = std::atoi(argv[++i]);
            } else if (arg == "-name") {
                array_name = argv[++i];
            }
        }
    }

    // ----------------------------------------------------------------
    // Validate
    // ----------------------------------------------------------------
    const bool has_pts = (pts_fn != nullptr);

    if (!found_input || !found_output || (!has_pts && !interactive_mode)) {
        std::cerr << "Generates a synthetic scar scalar field on a VTK PolyData mesh.\n"
                "\nUsage:\n"
                "  syntheticScar -i <mesh.vtk> -pts <ids.txt> -o <output.vtk> [options]\n"
                "  syntheticScar -i <mesh.vtk> --pick        -o <output.vtk> [options]\n"
                "\n(Mandatory)\n"
                "  -i   <mesh.vtk>     Input VTK PolyData mesh\n"
                "  -o   <output.vtk>   Output VTK mesh with scar scalar array\n"
                "  -pts <ids.txt>      Seed point ID file (one ID per line)\n"
                "                      OR use --pick for interactive selection\n"
                "\n(Optional)\n"
                "  -n       <int>      Neighbourhood hops for corridor spread (default: 15)\n"
                "  -sigma   <float>    Gaussian sigma in world units (default: auto)\n"
                "  -falloff <int>      1 = Gaussian (default), 2 = Linear\n"
                "  -name    <string>   Output scalar array name (default: synthetic_scar)\n"
                "  --pick              Interactive point picking mode\n"
             << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Load mesh
    // ----------------------------------------------------------------
    LaShell* source_mesh = new LaShell(input_fn);

    // ----------------------------------------------------------------
    // Interactive picking: open the viewer, let the user press 'x' to
    // mark points and 's' to save pointsIDlist.txt, then exit.
    // ----------------------------------------------------------------
    const char* picked_ids_file = "pointsIDlist.txt";

    if (interactive_mode) {
        std::cout << "\nInteractive mode:\n"
                "  Press 'x' to pick seed points on the mesh.\n"
                "  Press 's' to save point IDs and exit the viewer.\n"
                "  The scar will then be generated from those IDs.\n\n";

        LaShellGapsInBinary* picker = new LaShellGapsInBinary();
        picker->SetInputData(source_mesh);
        picker->Run();
        pts_fn = picked_ids_file;
    }

    // ----------------------------------------------------------------
    // Run scar generation
    // ----------------------------------------------------------------
    LaShellSyntheticScar* algorithm = new LaShellSyntheticScar();
    algorithm->SetInputData(source_mesh);
    algorithm->ReadPointIDFile(pts_fn);
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

    algorithm->Update();

    LaShell* output_mesh = algorithm->GetOutput();
    output_mesh->ExportVTK(output_fn);

    std::cout << "Saved: " << output_fn << endl;

    // delete algorithm;
    // delete source_mesh;

    return 0;
}