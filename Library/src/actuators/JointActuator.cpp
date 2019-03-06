//
//  JointActuator.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/JointActuator.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"

namespace sf
{

JointActuator::JointActuator(std::string uniqueName) : Actuator(uniqueName)
{
    fe = NULL;
    jId = 0;
    j = NULL;
}

ActuatorType JointActuator::getType()
{
    return ActuatorType::ACTUATOR_JOINT;
}

std::string JointActuator::getJointName()
{
    if(fe != NULL)
        return fe->getJointName(jId);
    else if(j != NULL)
        return j->getName();
    else
        return std::string("");
}

void JointActuator::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    if(multibody != NULL && jointId < multibody->getNumOfJoints())
    {
        fe = multibody;
        jId = jointId;
    }
}

void JointActuator::AttachToJoint(Joint* joint)
{
    if(joint != NULL)
        j = joint;
}
    
}
