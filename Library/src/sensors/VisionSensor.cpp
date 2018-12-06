//
//  VisionSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/VisionSensor.h"

#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

VisionSensor::VisionSensor(std::string uniqueName, Scalar frequency) : Sensor(uniqueName, frequency)
{
    attach = NULL;
    o2s = Transform::getIdentity();
}

VisionSensor::~VisionSensor()
{
}

Transform VisionSensor::getSensorFrame()
{
    if(attach != NULL)
        return attach->getOTransform() * o2s;
    else
        return o2s;
}

SensorType VisionSensor::getType()
{
    return SensorType::SENSOR_VISION;
}

void VisionSensor::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        o2s = origin;
        attach = multibody->getLink(linkId).solid;
    }
}

void VisionSensor::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        o2s = origin;
        attach = solid;
    }
}

}