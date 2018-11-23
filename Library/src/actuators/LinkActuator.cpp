//
//  LinkActuator.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/LinkActuator.h"

using namespace sf;

LinkActuator::LinkActuator(std::string uniqueName) : Actuator(uniqueName)
{
    attach = NULL;
    g2a = btTransform::getIdentity();
}

ActuatorType LinkActuator::getType()
{
    return ActuatorType::ACTUATOR_LINK;
}

void LinkActuator::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const btTransform& location)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        g2a = UnitSystem::SetTransform(location);
        attach = multibody->getLink(linkId).solid;
    }
}

void LinkActuator::AttachToSolid(SolidEntity* solid, const btTransform& location)
{
    if(solid != NULL)
    {
        g2a = UnitSystem::SetTransform(location);
        attach = solid;
    }
}


