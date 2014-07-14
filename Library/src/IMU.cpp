//
//  IMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "IMU.h"

#pragma mark Constructors
IMU::IMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = UnitSystem::SetTransform(relFrame);
    channels.push_back(SensorChannel("Roll", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Pitch", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Yaw", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Acceleration X", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Acceleration Y", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Acceleration Z", QUANTITY_ACCELERATION));
}