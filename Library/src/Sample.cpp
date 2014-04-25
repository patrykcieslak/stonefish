//
//  Sample.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Sample.h"
#include "SimulationApp.h"

Sample::Sample(ushort nDimensions, btScalar* values)
{
    nDim = nDimensions > 0 ? nDimensions : 1;
    data = new btScalar[nDim];
    std::memcpy(data, values, sizeof(btScalar)*nDim);
    timestamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
}

Sample::Sample(const Sample& other)
{
    timestamp = other.timestamp;
    nDim = other.nDim;
    data = new btScalar[nDim];
    std::memcpy(data, other.data, sizeof(btScalar)*nDim);
}

Sample::~Sample()
{
    delete[] data;
}

btScalar Sample::getTimestamp()
{
    return timestamp;
}

btScalar Sample::getValue(ushort dimension)
{
    if(dimension < nDim)
        return data[dimension];
    else
        return btScalar(0.);
}

std::vector<btScalar> Sample::getData()
{
    return std::vector<btScalar>(data, data+nDim);
}