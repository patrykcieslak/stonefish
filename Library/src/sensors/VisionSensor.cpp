//
//  VisionSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/VisionSensor.h"

using namespace sf;

VisionSensor::VisionSensor(std::string uniqueName, btScalar frequency) : Sensor(uniqueName, frequency)
{
    attach = NULL;
    g2s = btTransform::getIdentity();
}

VisionSensor::~VisionSensor()
{
}

btTransform VisionSensor::getSensorFrame()
{
    if(attach != NULL)
        return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    else
        return g2s;
}

SensorType VisionSensor::getType()
{
    return SensorType::SENSOR_VISION;
}

void VisionSensor::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const btTransform& location)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        g2s = UnitSystem::SetTransform(location);
        attach = multibody->getLink(linkId).solid;
    }
}

void VisionSensor::AttachToSolid(SolidEntity* solid, const btTransform& location)
{
    if(solid != NULL)
    {
        g2s = UnitSystem::SetTransform(location);
        attach = solid;
    }
}
