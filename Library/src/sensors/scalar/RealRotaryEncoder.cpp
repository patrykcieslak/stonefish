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
//  Copyright (c) 2014-2025 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RealRotaryEncoder.h"

#include "utils/UnitSystem.h"
#include "sensors/Sample.h"

namespace sf
{

RealRotaryEncoder::RealRotaryEncoder(std::string uniqueName, unsigned int cpr_resolution, bool absolute, Scalar frequency, int historyLength) : RotaryEncoder(uniqueName, frequency, historyLength)
{
    cprResolution_ = cpr_resolution;
    abs_ = absolute;
}

void RealRotaryEncoder::Reset()
{
    //incremental angle
    angle_ = lastAngle_ = GetRawAngle();
  
    //quantization
    lastAngle_ /= FULL_ANGLE;
    lastAngle_ = Scalar(trunc(lastAngle_ * cprResolution_)) / Scalar(cprResolution_) * FULL_ANGLE;
    
    ScalarSensor::Reset();
}

void RealRotaryEncoder::InternalUpdate(Scalar dt)
{
    if(abs_)
    {
        //to positive
        angle_ = GetRawAngle();
        if(angle_ < Scalar(0))
            angle_ += FULL_ANGLE;
        
        //quantization
        angle_ /= FULL_ANGLE;
        angle_ = Scalar(trunc(angle_ * Scalar((1 << cprResolution_) - 1))) / Scalar((1 << cprResolution_) - 1) * FULL_ANGLE;
    }
    else
    {
        //new angle
        Scalar actualAngle = GetRawAngle();
        
        //quantization
        actualAngle /= FULL_ANGLE;
        actualAngle = Scalar(trunc(actualAngle * cprResolution_)) / Scalar(cprResolution_) * FULL_ANGLE;
        
        //accumulate
        if(lastAngle_ * actualAngle < Scalar(0))
        {
            if(lastAngle_ > M_PI_4)
                angle_ += ((actualAngle + FULL_ANGLE) - lastAngle_);
            else if(lastAngle_ < -M_PI_4)
                angle_ += (actualAngle - (lastAngle_ + FULL_ANGLE));
            else
                angle_ += (actualAngle-lastAngle_);
        }
        else
            angle_ += (actualAngle-lastAngle_);
        
        lastAngle_ = actualAngle;
    }
    
    //record sample
    Sample s{std::vector<Scalar>({angle_, Scalar(0)})};
    AddSampleToHistory(s);
}

ScalarSensorType RealRotaryEncoder::getScalarSensorType() const
{
    return ScalarSensorType::ENCODER;
}

}
