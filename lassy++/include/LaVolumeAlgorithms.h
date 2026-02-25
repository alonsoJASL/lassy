/*
 *  LaVolumeAlgorithms.h
 *
 *  Abstract base class for algorithms operating on LaVolume objects.
 *  Peer of LaShellAlgorithms and LaImageAlgorithms.
 */
#pragma once

#include "LaVolume.h"


class LaVolumeAlgorithms {

public:

    virtual void SetInputData(LaVolume* volume);
    virtual void Update();
    virtual LaVolume* GetOutput();

protected:

    LaVolumeAlgorithms();
    ~LaVolumeAlgorithms();
};