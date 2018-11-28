//
//  RealRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RealRotaryEncoder.h"

#include "utils/UnitSystem.h"

using namespace sf;

RealRotaryEncoder::RealRotaryEncoder(std::string uniqueName, unsigned int cpr_resolution, bool absolute, Scalar frequency, int historyLength) : RotaryEncoder(uniqueName, frequency, historyLength)
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
    lastAngle = Scalar(trunc(lastAngle * cpr_res)) / Scalar(cpr_res) * FULL_ANGLE;
    
    ScalarSensor::Reset();
}

void RealRotaryEncoder::InternalUpdate(Scalar dt)
{
    if(abs)
    {
        //to positive
        angle = GetRawAngle();
        if(angle < Scalar(0))
            angle += FULL_ANGLE;
        
        //quantization
        angle /= FULL_ANGLE;
        angle = Scalar(trunc(angle * Scalar((1 << cpr_res) - 1))) / Scalar((1 << cpr_res) - 1) * FULL_ANGLE;
    }
    else
    {
        //new angle
        Scalar actualAngle = GetRawAngle();
        
        //quantization
        actualAngle /= FULL_ANGLE;
        actualAngle = Scalar(trunc(actualAngle * cpr_res)) / Scalar(cpr_res) * FULL_ANGLE;
        
        //accumulate
        if(lastAngle * actualAngle < Scalar(0))
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
    Scalar m[2];
    m[0] = angle;
    m[1] = Scalar(0.);
    
    Sample s(2, m);
    AddSampleToHistory(s);
}
