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
//  IMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/IMU.h"

#include "entities/SolidEntity.h"
#include "sensors/Sample.h"

namespace sf
{

IMU::IMU(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Roll", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Pitch", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Yaw", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QUANTITY_ANGULAR_VELOCITY));
}

void IMU::InternalUpdate(Scalar dt)
{
    //get sensor frame in world
    Transform imuTrans = getSensorFrame();
    
    //get angular velocity
    Vector3 av = imuTrans.getBasis().inverse() * attach->getAngularVelocity();
    
    //get angles
    Scalar yaw, pitch, roll;
    imuTrans.getBasis().getEulerYPR(yaw, pitch, roll);
    
    //record sample
    Scalar values[6] = {roll, pitch, yaw, av.x(), av.y(), av.z()};
    Sample s(6, values);
    AddSampleToHistory(s);
}

void IMU::setRange(Scalar angularVelocityMax)
{
    channels[3].rangeMin = -angularVelocityMax;
    channels[4].rangeMin = -angularVelocityMax;
    channels[5].rangeMin = -angularVelocityMax;
    channels[3].rangeMax = angularVelocityMax;
    channels[4].rangeMax = angularVelocityMax;
    channels[5].rangeMax = angularVelocityMax;
}
    
void IMU::setNoise(Scalar angleStdDev, Scalar angularVelocityStdDev)
{
    channels[0].setStdDev(angleStdDev);
    channels[1].setStdDev(angleStdDev);
    channels[2].setStdDev(angleStdDev);
    channels[3].setStdDev(angularVelocityStdDev);
    channels[4].setStdDev(angularVelocityStdDev);
    channels[5].setStdDev(angularVelocityStdDev);
}

ScalarSensorType IMU::getScalarSensorType()
{
    return ScalarSensorType::IMU;
}


}
