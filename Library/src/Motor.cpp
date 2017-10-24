//
//  Motor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2017 Patryk Cieslak. All rights reserved.
//

#include "Motor.h"

Motor::Motor(std::string uniqueName) : Actuator(uniqueName)
{
    revoluteOutput = NULL;
    multibodyOutput = NULL;
    multibodyJoint = 0;
    //Internal states
    torque = btScalar(0.);
}

ActuatorType Motor::getType()
{
    return ACTUATOR_MOTOR;
}

btScalar Motor::getTorque()
{
    return torque;
}

btScalar Motor::getAngle()
{
    if(revoluteOutput != NULL)
    {    
        return revoluteOutput->getAngle();
    }
    else if(multibodyOutput != NULL)
    {
        btScalar angle = btScalar(0);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibodyOutput->getJointPosition(multibodyJoint, angle, jt);
        
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
    btScalar angularV;
    
    if(revoluteOutput != NULL)
    {
        angularV = revoluteOutput->getAngularVelocity();
    }
    else if(multibodyOutput != NULL)
    {
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibodyOutput->getJointVelocity(multibodyJoint, angularV, jt);
        
        if(jt != btMultibodyLink::eRevolute)
            return btScalar(0);
    }
    else 
        return btScalar(0);
        
  /*  if(btFabs(angularV) < 1e-6)
        return btScalar(0);
    else*/
    return angularV;
}

void Motor::setIntensity(btScalar value)
{
    torque = value;
}

void Motor::AttachToJoint(RevoluteJoint* revolute)
{
    revoluteOutput = revolute;
    multibodyOutput = NULL;
    multibodyJoint = 0;
}

void Motor::AttachToJoint(FeatherstoneEntity* mb, unsigned int jointId)
{
    revoluteOutput = NULL;
    multibodyOutput = mb;
    multibodyJoint = jointId;
}

void Motor::Update(btScalar dt)
{
    //Drive the joint
    if(revoluteOutput != NULL)
        revoluteOutput->ApplyTorque(UnitSystem::GetTorque(torque));
    else if(multibodyOutput != NULL)
        multibodyOutput->DriveJoint(multibodyJoint, UnitSystem::GetTorque(torque));
}