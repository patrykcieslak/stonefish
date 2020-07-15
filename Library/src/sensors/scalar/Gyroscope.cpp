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
//  Gyroscope.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Gyroscope.h"

#include "entities/SolidEntity.h"
#include "sensors/scalar/ADC.h"
#include "sensors/Sample.h"

namespace sf
{

Gyroscope::Gyroscope(std::string uniqueName, Scalar rangeMin, Scalar rangeMax, Scalar sensitivity, Scalar zeroVoltage, Scalar driftSpeed, ADC* adc, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
    
    range[0] = rangeMin;
    range[1] = rangeMax;
    sens = sensitivity;
    drift = driftSpeed;

    zeroV = zeroVoltage;
    this->adc = adc;
}

void Gyroscope::Reset()
{
    accumulatedDrift = 0;
    ScalarSensor::Reset();
}

void Gyroscope::InternalUpdate(Scalar dt)
{
    //calculate transformation from global to gyro frame
    Matrix3 toGyroFrame = getSensorFrame().getBasis().inverse();
    
    //get angular velocity
    Vector3 actualAV = attach->getAngularVelocity();
    actualAV = toGyroFrame * actualAV;
    
    //select axis Z
    Scalar av = actualAV.getZ();
    
    //add limits/noise/nonlinearity/drift
    accumulatedDrift += drift * dt;
    av += accumulatedDrift;
    av = av < range[0] ? range[0] : (av > range[1] ? range[1] : av);
    
    //put through ADC
    av = (adc->MeasureVoltage(av * sens + zeroV) - zeroV) / sens; //sensitivity V/(rad/s)
    
    //save sample
    Sample s(1, &av);
    AddSampleToHistory(s);
}

ScalarSensorType Gyroscope::getScalarSensorType()
{
    return ScalarSensorType::GYRO;
}

}
