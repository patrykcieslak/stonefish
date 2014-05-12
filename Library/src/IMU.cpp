//
//  IMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "IMU.h"

IMU::IMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, unsigned int historyLength) : Sensor(uniqueName, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
}

void IMU::Reset()
{
    
}

void IMU::Update(btScalar dt)
{
    
}

unsigned short IMU::getNumOfDimensions()
{
    return 9; //acceleration, angular velocity, tilt
}
