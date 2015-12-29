//
//  Motor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015 Patryk Cieslak. All rights reserved.
//

#include "Motor.h"

#pragma mark Constructors
Motor::Motor(std::string uniqueName, RevoluteJoint* revolute) : Actuator(uniqueName)
{
    //Params
    revoluteOutput = revolute;
    multibodyOutput = NULL;
    multibodyChild = 0;
    
    //Internal states
    torque = btScalar(0.);
}

Motor::Motor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child) : Actuator(uniqueName)
{
    //Params
    revoluteOutput = NULL;
    multibodyOutput = mb;
    multibodyChild = child;
    
    //Internal states
    torque = btScalar(0.);
}

#pragma mark - Accessors
ActuatorType Motor::getType()
{
    return ACTUATOR_MOTOR;
}

btScalar Motor::getTorque()
{
    return torque;
}

btScalar Motor::getAngularVelocity()
{
    if(multibodyOutput == NULL)
    {
        return revoluteOutput->getAngularVelocity();
    }
    else
    {
        btScalar angularV = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibodyOutput->getJointVelocity(multibodyChild, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angularV;
        else
            return btScalar(0.);
    }
}

void Motor::setTorque(btScalar value)
{
    torque = value;
}

#pragma mark - Actuator
btVector3 Motor::Render()
{
    return btVector3(0.f,0.f,0.f);
}

void Motor::Update(btScalar dt)
{
    //Get joint angular velocity in radians
    btScalar aVelocity = getAngularVelocity();
    
    //Drive the joint
    if(multibodyOutput == NULL)
        revoluteOutput->ApplyTorque(UnitSystem::GetTorque(torque));
    else
        multibodyOutput->DriveJoint(multibodyChild, UnitSystem::GetTorque(torque));
}