//
//  RealRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RealRotaryEncoder.h"

using namespace sf;

RealRotaryEncoder::RealRotaryEncoder(std::string uniqueName, unsigned int cpr_resolution, bool absolute, btScalar frequency, int historyLength) : RotaryEncoder(uniqueName, frequency, historyLength)
{
    cpr_res = cpr_resolution;
    abs = absolute;
}

void RealRotaryEncoder::Reset()
{
    //incremental angle
    angle = lastAngle = GetRawAngle();
  
    //quantization
    lastAngle /= FULL_ANGLE;
    lastAngle = btScalar(trunc(lastAngle * cpr_res)) / btScalar(cpr_res) * FULL_ANGLE;
    
    ScalarSensor::Reset();
}

void RealRotaryEncoder::InternalUpdate(btScalar dt)
{
    if(abs)
    {
        //to positive
        angle = GetRawAngle();
        if(angle < btScalar(0))
            angle += FULL_ANGLE;
        
        //quantization
        angle /= FULL_ANGLE;
        angle = btScalar(trunc(angle * btScalar((1 << cpr_res) - 1))) / btScalar((1 << cpr_res) - 1) * FULL_ANGLE;
    }
    else
    {
        //new angle
        btScalar actualAngle = GetRawAngle();
        
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
    
    //record sample
    btScalar m[2];
    m[0] = angle;
    m[1] = btScalar(0.);
    
    Sample s(2, m);
    AddSampleToHistory(s);
}
