#define HAS_VTK 1

#include <iostream>
#include "../include/LaVolumeAlgorithms.h"

LaVolumeAlgorithms::LaVolumeAlgorithms() {}
LaVolumeAlgorithms::~LaVolumeAlgorithms() {}

void LaVolumeAlgorithms::SetInputData(LaVolume* volume) {
    std::cerr << "LaVolumeAlgorithms::SetInputData — not implemented in base class." << std::endl;
}

void LaVolumeAlgorithms::Update() {
    std::cerr << "LaVolumeAlgorithms::Update — not implemented in base class." << std::endl;
}

LaVolume* LaVolumeAlgorithms::GetOutput() {
    std::cerr << "LaVolumeAlgorithms::GetOutput — not implemented in base class." << std::endl;
    return nullptr;
}