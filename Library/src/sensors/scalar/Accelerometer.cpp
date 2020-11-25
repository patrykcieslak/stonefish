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
//  Accelerometer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Accelerometer.h"

#include "entities/MovingEntity.h"
#include "sensors/Sample.h"

namespace sf
{

Accelerometer::Accelerometer(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Acceleration X", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Acceleration Y", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Acceleration Z", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Angular acceleration X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular acceleration Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular acceleration Z", QUANTITY_ANGULAR_VELOCITY));
}

void Accelerometer::InternalUpdate(Scalar dt)
{
    //calculate transformation from global to imu frame
    Transform accTrans = getSensorFrame();
    
    //get acceleration
    Vector3 la = accTrans.getBasis().inverse() * (attach->getLinearAcceleration() + attach->getAngularAcceleration().cross(accTrans.getOrigin() - attach->getCGTransform().getOrigin()));
    
    //get angular acceleration
    Vector3 aa = accTrans.getBasis().inverse() * attach->getAngularAcceleration();
    
    //record sample
    Scalar values[6] = {la.x(), la.y(), la.z(), aa.x(), aa.y(), aa.z()};
    Sample s(6, values);
    AddSampleToHistory(s);
}

void Accelerometer::setRange(Scalar linearAccMax, Scalar angularAccMax)
{
    channels[0].rangeMin = -linearAccMax;
    channels[1].rangeMin = -linearAccMax;
    channels[2].rangeMin = -linearAccMax;
    channels[0].rangeMax = linearAccMax;
    channels[1].rangeMax = linearAccMax;
    channels[2].rangeMax = linearAccMax;
    
    channels[3].rangeMin = -angularAccMax;
    channels[4].rangeMin = -angularAccMax;
    channels[5].rangeMin = -angularAccMax;
    channels[3].rangeMax = angularAccMax;
    channels[4].rangeMax = angularAccMax;
    channels[5].rangeMax = angularAccMax;
}
    
void Accelerometer::setNoise(Scalar linearAccStdDev, Scalar angularAccStdDev)
{
    channels[0].setStdDev(linearAccStdDev);
    channels[1].setStdDev(linearAccStdDev);
    channels[2].setStdDev(linearAccStdDev);
    channels[3].setStdDev(angularAccStdDev);
    channels[4].setStdDev(angularAccStdDev);
    channels[5].setStdDev(angularAccStdDev);
}

ScalarSensorType Accelerometer::getScalarSensorType()
{
    return ScalarSensorType::ACC;
}
    
}
