//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = joint;
    multibody = NULL;
    multibodyJoint = 0;
    motor = NULL;
    thrust = NULL;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

RotaryEncoder::RotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = NULL;
    multibody = fe;
    multibodyJoint = joint;
    motor = NULL;
    thrust = NULL;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

RotaryEncoder::RotaryEncoder(std::string uniqueName, Motor* m, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = NULL;
    multibody = NULL;
    multibodyJoint = 0;
    motor = m;
    thrust = NULL;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

RotaryEncoder::RotaryEncoder(std::string uniqueName, Thruster* th, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = NULL;
    multibody = NULL;
    multibodyJoint = 0;
    motor = NULL;
    thrust = th;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

btScalar RotaryEncoder::GetRawAngle()
{
    if(revolute != NULL)
    {
        return revolute->getAngle();
    }
    else if(multibody != NULL)
    {
        btScalar mbAngle = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibody->getJointPosition(multibodyJoint, mbAngle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAngle;
        else
            return btScalar(0.);
    }
    else if(motor != NULL)
    {
        return motor->getAngle();
    }
    else if(thrust != NULL)
    {
        return thrust->getAngle();
    }
    else
        return btScalar(0);
}

btScalar RotaryEncoder::GetRawAngularVelocity()
{
    if(revolute != NULL)
    {
        return revolute->getAngularVelocity();
    }
    else if(multibody != NULL)
    {
        btScalar mbAV(0);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibody->getJointVelocity(multibodyJoint, mbAV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAV;
        else
            return btScalar(0);
    }
    else if(motor != NULL)
    {
        return motor->getAngularVelocity();
    }
    else if(thrust != NULL)
    {
        return thrust->getOmega();
    }
    else
        return btScalar(0);
}