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
//  Copyright (c) 2014-2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Gyroscope.h"

#include "entities/MovingEntity.h"
#include "sensors/Sample.h"

namespace sf
{

Gyroscope::Gyroscope(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Angular velocity X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QUANTITY_ANGULAR_VELOCITY));
    bias = Scalar(0);
}

void Gyroscope::InternalUpdate(Scalar dt)
{
    //calculate transformation from global to gyro frame
    Matrix3 toGyroFrame = getSensorFrame().getBasis().inverse();
    
    //get angular velocity
    Vector3 omega = toGyroFrame * attach->getAngularVelocity();

    //add bias error
    omega += Vector3(bias, bias, bias);

    //record sample
    Scalar values[3] = {omega.x(), omega.y(), omega.z()};
    Sample s(3, values);
    AddSampleToHistory(s);
}

void Gyroscope::setRange(Scalar angularVelMax)
{
    channels[0].rangeMin = -angularVelMax;
    channels[1].rangeMin = -angularVelMax;
    channels[2].rangeMin = -angularVelMax;
    channels[0].rangeMax = angularVelMax;
    channels[1].rangeMax = angularVelMax;
    channels[2].rangeMax = angularVelMax;
}

void Gyroscope::setNoise(Scalar angularVelStdDev, Scalar angularVelBias)
{
    channels[0].setStdDev(angularVelStdDev);
    channels[1].setStdDev(angularVelStdDev);
    channels[2].setStdDev(angularVelStdDev);
    bias = angularVelBias;
}

ScalarSensorType Gyroscope::getScalarSensorType()
{
    return ScalarSensorType::GYRO;
}

}
