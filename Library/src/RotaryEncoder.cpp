//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(std::string uniqueName, RevoluteJoint* joint, unsigned int cpr_resolution, bool absolute, unsigned int historyLength) : Sensor(uniqueName, historyLength)
{
    revolute = joint;
    cpr_res = cpr_resolution;
    abs = absolute;
    
    Reset();
}

void RotaryEncoder::Reset()
{
    //incremental angle
    angle = btScalar(0);
    lastAngle = UnitSystem::SetAngle(revolute->getAngle());
  
    //quantization
    lastAngle /= FULL_ANGLE;
    lastAngle = btScalar(trunc(lastAngle * cpr_res)) / btScalar(cpr_res) * FULL_ANGLE;
}

void RotaryEncoder::Update(btScalar dt)
{
    if(abs)
    {
        //to positive
        angle = UnitSystem::SetAngle(revolute->getAngle());
        if(angle < btScalar(0))
            angle += FULL_ANGLE;
        
        //quantization
        angle /= FULL_ANGLE;
        angle = btScalar(trunc(angle * btScalar((1 << cpr_res) - 1))) / btScalar((1 << cpr_res) - 1) * FULL_ANGLE;
    }
    else
    {
        //new angle
        btScalar actualAngle = UnitSystem::SetAngle(revolute->getAngle());
        
        //quantization
        actualAngle /= FULL_ANGLE;
        actualAngle = btScalar(trunc(actualAngle * cpr_res)) / btScalar(cpr_res) * FULL_ANGLE;
        
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
    }
    
    //save sample
    angle = UnitSystem::GetAngle(angle);
    Sample s(1, &angle);
    AddSampleToHistory(s);
}

unsigned short RotaryEncoder::getNumOfDimensions()
{
    return 1;
}