//
//  LinkSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/LinkSensor.h"

#include "utils/MathUtil.hpp"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

LinkSensor::LinkSensor(std::string uniqueName, Scalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    attach = NULL;
    o2s = Transform::getIdentity();
}

LinkSensor::~LinkSensor()
{
}

Transform LinkSensor::getSensorFrame()
{
    if(attach != NULL)
        return attach->getOTransform() * o2s;
    else
        return o2s;
}

SensorType LinkSensor::getType()
{
    return SensorType::SENSOR_LINK;
}

void LinkSensor::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        o2s = origin;
        attach = multibody->getLink(linkId).solid;
    }
}

void LinkSensor::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        o2s = origin;
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

}
