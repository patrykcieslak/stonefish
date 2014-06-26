//
//  IMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "IMU.h"

IMU::IMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
}

void IMU::Reset()
{
    Sensor::Reset();
}

void IMU::InternalUpdate(btScalar dt)
{
}

unsigned short IMU::getNumOfDimensions()
{
    return 9; //acceleration, angular velocity, tilt
}
