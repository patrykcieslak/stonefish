//
//  TrueRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "TrueRotaryEncoder.h"

TrueRotaryEncoder::TrueRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, unsigned int cpr_resolution, bool absolute, btScalar frequency, unsigned int historyLength) : RotaryEncoder(uniqueName, joint, frequency, historyLength)
{
    cpr_res = cpr_resolution;
    abs = absolute;
}

TrueRotaryEncoder::TrueRotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, unsigned int cpr_resolution, bool absolute, btScalar frequency, unsigned int historyLength) : RotaryEncoder(uniqueName, fe, joint, frequency, historyLength)
{
    cpr_res = cpr_resolution;
    abs = absolute;
}

void TrueRotaryEncoder::Reset()
{
    //incremental angle
    angle = revolute->getAngle();
    lastAngle = revolute->getAngle();
  
    //quantization
    lastAngle /= FULL_ANGLE;
    lastAngle = btScalar(trunc(lastAngle * cpr_res)) / btScalar(cpr_res) * FULL_ANGLE;
    
    SimpleSensor::Reset();
}

void TrueRotaryEncoder::InternalUpdate(btScalar dt)
{
    if(abs)
    {
        //to positive
        angle = revolute->getAngle();
        if(angle < btScalar(0))
            angle += FULL_ANGLE;
        
        //quantization
        angle /= FULL_ANGLE;
        angle = btScalar(trunc(angle * btScalar((1 << cpr_res) - 1))) / btScalar((1 << cpr_res) - 1) * FULL_ANGLE;
    }
    else
    {
        //new angle
        btScalar actualAngle = revolute->getAngle();
        
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
    btScalar m[2];
    m[0] = angle;
    m[1] = btScalar(0.);
    
    Sample s(2, m);
    AddSampleToHistory(s);
}