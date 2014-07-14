//
//  Magnetometer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Magnetometer.h"

#pragma mark Constructors
Magnetometer::Magnetometer(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, unsigned short resolution, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
    axis = senseAxis;
    bits = resolution;
    
    std::string axisName[3] = {"X", "Y", "Z"};
    channels.push_back(SensorChannel("Magnetic field " + axisName[axis], QUANTITY_UNITLESS));
}

#pragma mark - Methods
void Magnetometer::Reset()
{
    Sensor::Reset();
}

void Magnetometer::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to acc frame
    btMatrix3x3 toMagFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //magnetic north
    btVector3 north = toMagFrame * btVector3(1, 0, 0);
    
    //select axis
    btScalar mag = north[axis];
    
    //quantization
    mag = btScalar(trunc(mag * btScalar((1 << bits) - 1))) / btScalar((1 << bits) - 1); //quantization
    
    //save sample
    Sample s(1, &mag);
    AddSampleToHistory(s);
}