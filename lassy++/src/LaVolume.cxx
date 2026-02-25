#define HAS_VTK 1

#include <iostream>
#include <string>
#include <algorithm>

#include "../include/LaVolume.h"



// ============================================================
// Internal helpers
// ============================================================

bool LaVolume::IsXMLFormat(const std::string& filename) {
    if (filename.size() < 4) {
        std::cerr << "LaVolume: unrecognised file extension: " << filename << std::endl;
        return false;
    }
    std::string ext = filename.substr(filename.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".vtu") return true;
    if (ext == ".vtk") return false;
    std::cerr << "LaVolume: unrecognised extension '" << ext
         << "'. Assuming legacy .vtk format." << std::endl;
    return false;
}


// ============================================================
// Constructors
// ============================================================

LaVolume::LaVolume() {
    _grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
}

LaVolume::LaVolume(const char* filename) {
    _grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    ImportFile(filename);
}


// ============================================================
// Grid access
// ============================================================

void LaVolume::SetGrid(vtkSmartPointer<vtkUnstructuredGrid> grid) {
    _grid = grid;
}

void LaVolume::GetGrid(vtkSmartPointer<vtkUnstructuredGrid> grid_out) const {
    grid_out->DeepCopy(_grid);
}

vtkUnstructuredGrid* LaVolume::GetGridPointer() const {
    return _grid.GetPointer();
}


// ============================================================
// Metadata
// ============================================================

vtkIdType LaVolume::GetNumberOfPoints() const {
    return _grid ? _grid->GetNumberOfPoints() : 0;
}

vtkIdType LaVolume::GetNumberOfCells() const {
    return _grid ? _grid->GetNumberOfCells() : 0;
}


// ============================================================
// I/O
// ============================================================

void LaVolume::ImportFile(const char* filename) {
    const std::string fn(filename);
    if (IsXMLFormat(fn)) {
        vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
            vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        reader->SetFileName(filename);
        reader->Update();
        _grid->DeepCopy(reader->GetOutput());
        std::cout << "LaVolume: loaded " << fn
             << " (" << _grid->GetNumberOfPoints() << " points, "
             << _grid->GetNumberOfCells() << " cells)" << std::endl;
    } else {
        vtkSmartPointer<vtkUnstructuredGridReader> reader =
            vtkSmartPointer<vtkUnstructuredGridReader>::New();
        reader->SetFileName(filename);
        reader->Update();
        _grid->DeepCopy(reader->GetOutput());
        std::cout << "LaVolume: loaded " << fn
             << " (" << _grid->GetNumberOfPoints() << " points, "
             << _grid->GetNumberOfCells() << " cells)" << std::endl;
    }
}

void LaVolume::ExportVTK(const char* filename) const {
    vtkSmartPointer<vtkUnstructuredGridWriter> writer =
        vtkSmartPointer<vtkUnstructuredGridWriter>::New();
    writer->SetFileName(filename);
    writer->SetInputData(_grid);
    writer->Write();
    std::cout << "LaVolume: saved " << filename << std::endl;
}

void LaVolume::ExportVTU(const char* filename) const {
    vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
        vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetFileName(filename);
    writer->SetInputData(_grid);
    writer->Write();
    std::cout << "LaVolume: saved " << filename << std::endl;
}


// ============================================================
// Conversion
// ============================================================

LaShell* LaVolume::ConvertToSurface() const {
    vtkSmartPointer<vtkDataSetSurfaceFilter> surface_filter =
        vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surface_filter->SetInputData(_grid);
    surface_filter->Update();

    LaShell* shell = new LaShell();
    shell->SetMesh3D(surface_filter->GetOutput());
    return shell;
}