//
//  FakeRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FakeRotaryEncoder.h"

FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, unsigned int historyLength) : Sensor(uniqueName, historyLength)
{
    revolute = joint;
    Reset();
}

void FakeRotaryEncoder::Reset()
{
    angle = btScalar(0);
    lastAngle = UnitSystem::SetAngle(revolute->getAngle());
    angularVelocity = btScalar(0);
}

void FakeRotaryEncoder::Update(btScalar dt)
{
    //new angle
    btScalar actualAngle = UnitSystem::SetAngle(revolute->getAngle());
        
    //accumulate
    if(lastAngle * actualAngle < btScalar(0))
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
    angularVelocity = UnitSystem::SetAngle(revolute->getAngularVelocity());
    
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
