//
//  FakeRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include <sensors/FakeRotaryEncoder.h>

FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency, int historyLength) : RotaryEncoder(uniqueName, joint, frequency, historyLength)
{
    Reset();
}

FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, FeatherstoneEntity* mb, unsigned int joint, btScalar frequency, int historyLength) : RotaryEncoder(uniqueName, mb, joint, frequency, historyLength)
{
    Reset();
}

FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, Motor* m, btScalar frequency, int historyLength) : RotaryEncoder(uniqueName, m, frequency, historyLength)
{
    Reset();
}

FakeRotaryEncoder::FakeRotaryEncoder(std::string uniqueName, Thruster* th, btScalar frequency, int historyLength) : RotaryEncoder(uniqueName, th, frequency, historyLength)
{
    Reset();
}

void FakeRotaryEncoder::Reset()
{
    angle = lastAngle = GetRawAngle();
    angularVelocity = GetRawAngularVelocity();
    
    SimpleSensor::Reset();
}

void FakeRotaryEncoder::InternalUpdate(btScalar dt)
{
    //new angle
    btScalar actualAngle = GetRawAngle();
        
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
    angularVelocity = GetRawAngularVelocity();
    
    //save sample
    btScalar m[2];
    m[0] = angle;
    m[1] = angularVelocity;
    
    Sample s(2, m);
    AddSampleToHistory(s);
}