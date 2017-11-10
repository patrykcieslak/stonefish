//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = joint;
    multibody = NULL;
    multibodyJoint = 0;
    motor = NULL;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

RotaryEncoder::RotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = NULL;
    multibody = fe;
    multibodyJoint = joint;
    motor = NULL;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

RotaryEncoder::RotaryEncoder(std::string uniqueName, Motor* m, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    revolute = NULL;
    multibody = NULL;
    multibodyJoint = 0;
    motor = m;
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
    else 
        return btScalar(0);
}