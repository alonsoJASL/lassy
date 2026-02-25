/*
 *  LaVolume.h
 *
 *  Encapsulates a volumetric mesh stored as a vtkUnstructuredGrid.
 *  Peer of LaShell (vtkPolyData) and LaImage (ITK image).
 *
 *  Supported I/O:
 *    .vtk  — legacy VTK unstructured grid format
 *    .vtu  — VTK XML unstructured grid format
 *
 *  CARP (.pts + .elem) support is deferred to a future pass.
 */
#pragma once
#define HAS_VTK 1

#include <memory>
#include <string>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkSmartPointer.h>

#include "LaShell.h"

class LaVolume {

private:
    vtkSmartPointer<vtkUnstructuredGrid> _grid;

    /*
     * Detects file format from extension.
     * Returns true for .vtu, false for .vtk.
     * Aborts with an error message for unrecognised extensions.
     */
    static bool IsXMLFormat(const std::string &filename);

public:
    LaVolume();

    /*
     * Loads a volumetric mesh from file.
     * Supported extensions: .vtk, .vtu
     */
    explicit LaVolume(const char *filename);

    // ------------------------------------------------------------------
    // Grid access  (mirrors LaShell::GetMesh3D / SetMesh3D)
    // ------------------------------------------------------------------

    void SetGrid(vtkSmartPointer<vtkUnstructuredGrid> grid);
    void GetGrid(vtkSmartPointer<vtkUnstructuredGrid> grid_out) const;

    /*
     * Direct pointer access — use when passing to VTK filters.
     * Does not transfer ownership.
     */
    vtkUnstructuredGrid *GetGridPointer() const;

    // ------------------------------------------------------------------
    // Metadata
    // ------------------------------------------------------------------

    vtkIdType GetNumberOfPoints() const;
    vtkIdType GetNumberOfCells() const;

    // ------------------------------------------------------------------
    // I/O
    // ------------------------------------------------------------------

    void ImportFile(const char *filename);
    void ExportVTK(const char *filename) const;
    void ExportVTU(const char *filename) const;

    // ------------------------------------------------------------------
    // Conversion
    // ------------------------------------------------------------------

    /*
     * Extracts the outer surface of the volumetric mesh using
     * vtkDataSetSurfaceFilter and returns it as a LaShell.
     * Replaces the ugrid2vtk application workflow programmatically.
     * Caller owns the returned pointer.
     */
    LaShell *ConvertToSurface() const;
};