/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  RealRotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RealRotaryEncoder.h"

#include "utils/UnitSystem.h"
#include "sensors/Sample.h"

namespace sf
{

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

ScalarSensorType RealRotaryEncoder::getScalarSensorType()
{
    return ScalarSensorType::ENCODER;
}

}
