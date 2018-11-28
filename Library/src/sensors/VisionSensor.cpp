//
//  VisionSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/VisionSensor.h"

using namespace sf;

VisionSensor::VisionSensor(std::string uniqueName, Scalar frequency) : Sensor(uniqueName, frequency)
{
    attach = NULL;
    g2s = Transform::getIdentity();
}

VisionSensor::~VisionSensor()
{
}

Transform VisionSensor::getSensorFrame()
{
    if(attach != NULL)
        return attach->getCGTransform() * attach->getG2CGTransform().inverse() * g2s;
    else
        return g2s;
}

SensorType VisionSensor::getType()
{
    return SensorType::SENSOR_VISION;
}

void VisionSensor::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        g2s = origin;
        attach = multibody->getLink(linkId).solid;
    }
}

void VisionSensor::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        g2s = origin;
        attach = solid;
    }
}
