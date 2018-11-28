//
//  Sample.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "sensors/Sample.h"

#include "core/SimulationApp.h"

using namespace sf;

Sample::Sample(unsigned short nDimensions, Scalar* values)
{
    nDim = nDimensions > 0 ? nDimensions : 1;
    data = new Scalar[nDim];
    std::memcpy(data, values, sizeof(Scalar)*nDim);
    timestamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
}

Sample::Sample(const Sample& other)
{
    timestamp = other.timestamp;
    nDim = other.nDim;
    data = new Scalar[nDim];
    std::memcpy(data, other.data, sizeof(Scalar)*nDim);
}

Sample::~Sample()
{
    delete [] data;
}

Scalar Sample::getTimestamp()
{
    return timestamp;
}

Scalar Sample::getValue(unsigned short dimension)
{
    if((dimension < nDim) && (data != NULL))
        return data[dimension];
    else
        return Scalar(0.);
}

std::vector<Scalar> Sample::getData()
{
    return std::vector<Scalar>(data, data+nDim);
}
