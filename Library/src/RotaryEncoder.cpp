//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RotaryEncoder.h"

#pragma mark Constructors
RotaryEncoder::RotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    revolute = joint;
    multibody = NULL;
    multibodyJoint = 0;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

RotaryEncoder::RotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    revolute = NULL;
    multibody = fe;
    multibodyJoint = joint;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

#pragma mark - Internal
btScalar RotaryEncoder::GetRawAngle()
{
    if(multibody == NULL)
    {
        return revolute->getAngle();
    }
    else
    {
        btScalar mbAngle = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibody->getJointPosition(multibodyJoint, mbAngle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAngle;
        else
            return btScalar(0.);
    }
}