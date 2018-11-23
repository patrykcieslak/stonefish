//
//  Motor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/Motor.h"

#include "joints/RevoluteJoint.h"

using namespace sf;

Motor::Motor(std::string uniqueName) : JointActuator(uniqueName)
{
    torque = btScalar(0);
}

void Motor::setIntensity(btScalar value)
{
    torque = value;
}

btScalar Motor::getTorque()
{
    return torque;
}

btScalar Motor::getAngle()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngle();
    }
    else if(fe != NULL)
    {
        btScalar angle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointPosition(jId, angle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angle;
        else
            return btScalar(0);
    }
    else
        return btScalar(0);
}

btScalar Motor::getAngularVelocity()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngularVelocity();
    }
    else if(fe != NULL)
    {
        btScalar angularV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointVelocity(jId, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angularV;
        else
            return btScalar(0);
    }
    else
        return btScalar(0);
}

void Motor::Update(btScalar dt)
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
        ((RevoluteJoint*)j)->ApplyTorque(UnitSystem::GetTorque(torque));
    else if(fe != NULL)
        fe->DriveJoint(jId, UnitSystem::GetTorque(torque));
}
