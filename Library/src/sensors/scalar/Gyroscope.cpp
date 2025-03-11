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
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Gyroscope.h"

#include "entities/MovingEntity.h"
#include "sensors/Sample.h"

namespace sf
{

Gyroscope::Gyroscope(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Angular velocity X", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QuantityType::ANGULAR_VELOCITY));
    bias = V0();
}

void Gyroscope::InternalUpdate(Scalar dt)
{
    //calculate transformation from global to gyro frame
    Matrix3 toGyroFrame = getSensorFrame().getBasis().inverse();
    
    //get angular velocity
    Vector3 omega = toGyroFrame * attach->getAngularVelocity();

    //add bias error
    omega += bias;

    //record sample
    Scalar values[3] = {omega.x(), omega.y(), omega.z()};
    Sample s(3, values);
    AddSampleToHistory(s);
}

void Gyroscope::setRange(Vector3 angularVelocityMax)
{
    channels[0].rangeMin = -btClamped(angularVelocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMin = -btClamped(angularVelocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[2].rangeMin = -btClamped(angularVelocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[0].rangeMax = btClamped(angularVelocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMax = btClamped(angularVelocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[2].rangeMax = btClamped(angularVelocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Gyroscope::setNoise(Vector3 angularVelocityStdDev, Vector3 angularVelocityBias)
{
    channels[0].setStdDev(btClamped(angularVelocityStdDev.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[1].setStdDev(btClamped(angularVelocityStdDev.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[2].setStdDev(btClamped(angularVelocityStdDev.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    bias = angularVelocityBias;
}

ScalarSensorType Gyroscope::getScalarSensorType()
{
    return ScalarSensorType::GYRO;
}

}
