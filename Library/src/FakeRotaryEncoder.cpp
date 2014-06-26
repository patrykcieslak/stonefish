//
//  FakeRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FakeRotaryEncoder.h"

#pragma mark Constructors
FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    revolute = joint;
    multibody = NULL;
    multibodyChild = 0;
    Reset();
}

FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    revolute = NULL;
    multibody = mb;
    multibodyChild = child;
    Reset();
}

#pragma mark - Sensor
void FakeRotaryEncoder::Reset()
{
    Sensor::Reset();
    angle = btScalar(0.);
    lastAngle = getRawAngle();
    angularVelocity = btScalar(0.);
}

void FakeRotaryEncoder::InternalUpdate(btScalar dt)
{
    //new angle
    btScalar actualAngle = getRawAngle();
        
    //accumulate
    if(lastAngle * actualAngle < btScalar(0.))
    {
        if(lastAngle > M_PI_4)
            angle += ((actualAngle + FULL_ANGLE) - lastAngle);
        else if(lastAngle < -M_PI_4)
            angle += (actualAngle - (lastAngle + FULL_ANGLE));
        else
            angle += (actualAngle-lastAngle);
    }
    else
        angle += (actualAngle-lastAngle);
    
    lastAngle = actualAngle;
    
    //angular velocity
    angularVelocity = getRawAngularVelocity();
    
    //save sample
    btScalar ext[2];
    ext[0] = UnitSystem::GetAngle(angle);
    ext[1] = UnitSystem::GetAngle(angularVelocity);
    
    Sample s(2, ext);
    AddSampleToHistory(s);
}

unsigned short FakeRotaryEncoder::getNumOfDimensions()
{
    return 2;
}

#pragma mark - Encoder
btScalar FakeRotaryEncoder::getRawAngle()
{
    if(multibody == NULL)
    {
        return UnitSystem::SetAngle(revolute->getAngle());
    }
    else
    {
        btScalar angle = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibody->getJointPosition(multibodyChild, angle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return UnitSystem::SetAngle(angle);
        else
            return btScalar(0.);
    }
}

btScalar FakeRotaryEncoder::getRawAngularVelocity()
{
    if(multibody == NULL)
    {
        return UnitSystem::SetAngle(revolute->getAngularVelocity());
    }
    else
    {
        btScalar angularV = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibody->getJointVelocity(multibodyChild, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return UnitSystem::SetAngle(angularV);
        else
            return btScalar(0.);
    }
}