#define HAS_VTK 1

#include "LaVolume.h"
#include "LaVolumeSyntheticScar.h"
#include "LaShell.h"
#include "LaShellGapsInBinary.h"

#include <iostream>
#include <string>
#include <vector>

/*
 *  Author:
 *  Dr. José Alonso Solís-Lemus
 *
 *  Generates a synthetic scar scalar field on a volumetric mesh (LaVolume).
 *
 *  Seeds can be provided in two ways:
 *
 *    --file   Non-interactive.  Reads seed IDs from a text file.
 *             If --surface is also provided, IDs are interpreted as surface
 *             point IDs and mapped to the nearest volumetric nodes.
 *             Without --surface, IDs are interpreted as volumetric node IDs.
 *
 *    --pick   Interactive.  Extracts the surface from the volume, opens the
 *             VTK viewer for point picking (press 'x'), saves IDs on 's',
 *             then maps picked surface IDs to volumetric nodes automatically.
 */
int main(int argc, char* argv[]) {
    const char* input_fn   = nullptr;
    const char* pts_fn     = nullptr;
    const char* output_fn  = nullptr;
    const char* array_name = nullptr;
    int    neighbourhood   = 15;
    double sigma           = -1.0;
    int    falloff_mode    = 1;
    bool   interactive     = false;
    bool   ids_are_surface = false;  // true when --surface flag present

    bool found_input  = false;
    bool found_output = false;

    if (argc < 2) {
        std::cerr << "syntheticScarVolume: too few arguments.\n";
    } else {
        for (int i = 1; i < argc; ++i) {
            const std::string arg(argv[i]);

            if (arg == "--pick") {
                interactive     = true;
                ids_are_surface = true;
                continue;
            }
            if (arg == "--surface") {
                ids_are_surface = true;
                continue;
            }

            if (i + 1 == argc) continue;

            if (arg == "-i") {
                input_fn    = argv[++i];
                found_input = true;
            } else if (arg == "-pts") {
                pts_fn = argv[++i];
            } else if (arg == "-o") {
                output_fn    = argv[++i];
                found_output = true;
            } else if (arg == "-n") {
                neighbourhood = atoi(argv[++i]);
            } else if (arg == "-sigma") {
                sigma = std::atof(argv[++i]);
            } else if (arg == "-falloff") {
                falloff_mode = std::atoi(argv[++i]);
            } else if (arg == "-name") {
                array_name = argv[++i];
            }
        }
    }

    const bool has_pts = (pts_fn != nullptr);

    if (!found_input || !found_output || (!has_pts && !interactive)) {
        std::cerr << "Generates a synthetic scar scalar field on a volumetric mesh.\n"
                "\nUsage:\n"
                "  syntheticScarVolume -i <mesh.vtk|vtu> -pts <ids.txt> -o <out.vtk> [options]\n"
                "  syntheticScarVolume -i <mesh.vtk|vtu> --pick         -o <out.vtk> [options]\n"
                "\n(Mandatory)\n"
                "  -i   <mesh>       Input volumetric mesh (.vtk or .vtu)\n"
                "  -o   <output>     Output mesh (.vtk or .vtu)\n"
                "  -pts <ids.txt>    Seed ID file (one ID per line)\n"
                "                    OR use --pick for interactive picking\n"
                "\n(Optional)\n"
                "  -n        <int>   Neighbourhood hops (default: 15)\n"
                "  -sigma    <float> Gaussian sigma in world units (default: auto)\n"
                "  -falloff  <int>   1=Gaussian (default), 2=Linear\n"
                "  -name     <str>   Output array name (default: synthetic_scar)\n"
                "  --surface         Treat -pts IDs as surface point IDs and map\n"
                "                    to nearest volumetric nodes\n"
                "  --pick            Interactive surface picking (implies --surface)\n"
             << std::endl;
        return 1;
    }

    // ----------------------------------------------------------------
    // Load volume
    // ----------------------------------------------------------------
    LaVolume* volume = new LaVolume(input_fn);

    // ----------------------------------------------------------------
    // Interactive picking on extracted surface
    // ----------------------------------------------------------------
    // const char* picked_ids_file = "poizntsIDlist.txt";
    std::string picked_ids_path = std::string(output_fn) + "_seedIDs.txt";
    const char *picked_ids_file = picked_ids_path.c_str();

    LaShell* surface_shell = nullptr;

    if (interactive) {
        surface_shell = volume->ConvertToSurface();

        std::cout << "\nInteractive mode (picking on extracted surface):\n"
                "  Press 'x' to pick seed points.\n"
                "  Press 's' to save and exit.\n\n";

        LaShellGapsInBinary* picker = new LaShellGapsInBinary();
        picker->SetInputData(surface_shell);
        picker->Run();

        std::cout << "Looking for IDs file: " << pts_fn << std::endl;
        std::ifstream test(pts_fn);
        std::cout << "File opens: " << (test.is_open() ? "yes" : "no") << std::endl;

        pts_fn = picked_ids_file;
        // picker intentionally not deleted — see C2 note in next-pass list
    }

    // ----------------------------------------------------------------
    // Run scar generation
    // ----------------------------------------------------------------
    LaVolumeSyntheticScar* algorithm = new LaVolumeSyntheticScar();
    algorithm->SetInputData(volume);

    if (ids_are_surface) {
        // Need the surface shell for coordinate mapping
        if (!surface_shell) {
            surface_shell = volume->ConvertToSurface();
        }

        // Read the ID file into a plain vector<int> for SetSurfaceSeeds
        std::vector<int> surface_ids;
        std::ifstream in(pts_fn);
        std::string line;
        while (getline(in, line)) {
            if (!line.empty()) {
                try { surface_ids.push_back(stoi(line)); }
                catch (...) {}
            }
        }
        algorithm->SetSurfaceSeeds(surface_shell, surface_ids);
    } else {
        algorithm->ReadPointIDFile(pts_fn);
    }

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

    LaVolume* output = algorithm->GetOutput();

    // Detect output format from extension
    const std::string out_str(output_fn);
    if (out_str.size() >= 4 &&
        out_str.substr(out_str.size() - 4) == ".vtu") {
        output->ExportVTU(output_fn);
    } else {
        output->ExportVTK(output_fn);
    }

    delete algorithm;
    delete volume;
    delete surface_shell;

    return 0;
}