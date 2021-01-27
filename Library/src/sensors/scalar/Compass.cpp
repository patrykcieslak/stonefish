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
//  Compass.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Compass.h"

#include "sensors/Sample.h"

namespace sf
{

Compass::Compass(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Heading", QuantityType::ANGLE));
}

void Compass::InternalUpdate(Scalar dt)
{
    //get angles
    Scalar yaw, pitch, roll;
    getSensorFrame().getBasis().getEulerYPR(yaw, pitch, roll);
    
    //record sample
    Sample s(1, &yaw);
    AddSampleToHistory(s);
}

void Compass::setNoise(Scalar headingStdDev)
{
    channels[0].setStdDev(btClamped(headingStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Compass::getScalarSensorType()
{
    return ScalarSensorType::COMPASS;
}

}
