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
    o2a = Transform::getIdentity();
}

ActuatorType LinkActuator::getType()
{
    return ActuatorType::ACTUATOR_LINK;
}
    
Transform LinkActuator::getActuatorFrame()
{
    if(attach != NULL)
        return attach->getOTransform() * o2a;
    else
        return o2a;
}

void LinkActuator::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        o2a = origin;
        attach = multibody->getLink(linkId).solid;
    }
}

void LinkActuator::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        o2a = origin;
        attach = solid;
    }
}
    
}
