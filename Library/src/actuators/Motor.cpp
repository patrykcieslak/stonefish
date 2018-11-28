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
    torque = Scalar(0);
}

void Motor::setIntensity(Scalar value)
{
    torque = value;
}

Scalar Motor::getTorque()
{
    return torque;
}

Scalar Motor::getAngle()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngle();
    }
    else if(fe != NULL)
    {
        Scalar angle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointPosition(jId, angle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angle;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

Scalar Motor::getAngularVelocity()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngularVelocity();
    }
    else if(fe != NULL)
    {
        Scalar angularV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointVelocity(jId, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angularV;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

void Motor::Update(Scalar dt)
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
        ((RevoluteJoint*)j)->ApplyTorque(torque);
    else if(fe != NULL)
        fe->DriveJoint(jId, torque);
}
