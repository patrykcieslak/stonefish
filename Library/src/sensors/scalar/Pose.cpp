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
//  Pose.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014-2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Pose.h"

#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

Pose::Pose(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Coordinate X", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Coordinate Y", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Coordinate Z", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Roll", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Pitch", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Yaw", QuantityType::ANGLE));
}

void Pose::InternalUpdate(Scalar dt)
{
    //get angles
    Transform trajFrame = getSensorFrame();
    Scalar yaw, pitch, roll;
    trajFrame.getBasis().getEulerYPR(yaw, pitch, roll);
    
    //record sample
    Scalar values[6] = {trajFrame.getOrigin().x(), trajFrame.getOrigin().y(), trajFrame.getOrigin().z(), roll, pitch, yaw};
    Sample s(6, values);
    AddSampleToHistory(s);
}

ScalarSensorType Pose::getScalarSensorType()
{
    return ScalarSensorType::POSE;
}

}
