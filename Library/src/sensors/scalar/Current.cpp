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
//  Current.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Current.h"

#include "actuators/DCMotor.h"
#include "sensors/Sample.h"

namespace sf
{

Current::Current(std::string uniqueName, Scalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    motor = NULL;
    channels.push_back(SensorChannel("Current", QuantityType::CURRENT));
}

void Current::AttachToMotor(DCMotor* m)
{
    if(m != NULL)
        motor = m;
}

void Current::InternalUpdate(Scalar dt)
{
    //read current
    Scalar current = Scalar(0);
    if(motor != NULL)
        current = motor->getCurrent();
    
    //record sample
    Sample s(1, &current);
    AddSampleToHistory(s);
}

SensorType Current::getType()
{
    return SensorType::OTHER;
}

ScalarSensorType Current::getScalarSensorType()
{
    return ScalarSensorType::CURRENT;
}

}
