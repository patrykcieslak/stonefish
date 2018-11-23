//
//  LinkSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/LinkSensor.h"

#include "utils/MathUtil.hpp"

using namespace sf;

LinkSensor::LinkSensor(std::string uniqueName, btScalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    attach = NULL;
    g2s = btTransform::getIdentity();
}

LinkSensor::~LinkSensor()
{
}

btTransform LinkSensor::getSensorFrame()
{
    if(attach != NULL)
        return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    else
        return g2s;
}

SensorType LinkSensor::getType()
{
    return SensorType::SENSOR_LINK;
}

void LinkSensor::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const btTransform& location)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        g2s = UnitSystem::SetTransform(location);
        attach = multibody->getLink(linkId).solid;
    }
}

void LinkSensor::AttachToSolid(SolidEntity* solid, const btTransform& location)
{
    if(solid != NULL)
    {
        g2s = UnitSystem::SetTransform(location);
        attach = solid;
    }
}

std::vector<Renderable> LinkSensor::Render()
{
    std::vector<Renderable> items(0);

    Renderable item;
    item.type = RenderableType::SENSOR_CS;
    item.model = glMatrixFromBtTransform(getSensorFrame());
    items.push_back(item);

    return items;
}
