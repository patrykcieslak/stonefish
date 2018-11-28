//
//  LinkActuator.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/LinkActuator.h"

#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

LinkActuator::LinkActuator(std::string uniqueName) : Actuator(uniqueName)
{
    attach = NULL;
    g2a = Transform::getIdentity();
}

ActuatorType LinkActuator::getType()
{
    return ActuatorType::ACTUATOR_LINK;
}

void LinkActuator::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        g2a = origin;
        attach = multibody->getLink(linkId).solid;
    }
}

void LinkActuator::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        g2a = origin;
        attach = solid;
    }
}

}
