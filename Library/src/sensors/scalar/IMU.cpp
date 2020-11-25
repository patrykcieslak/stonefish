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
//  Copyright (c) 2014-2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/IMU.h"

#include "entities/MovingEntity.h"
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
    yawDriftRate = Scalar(0);
    accumulatedYawDrift = Scalar(0);
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
    
    //accumulate and add drift
    accumulatedYawDrift += yawDriftRate * dt;
    yaw += accumulatedYawDrift;

    //record sample
    Scalar values[6] = {roll, pitch, yaw, av.x(), av.y(), av.z()};
    Sample s(6, values);
    AddSampleToHistory(s);
}

void IMU::Reset()
{
    ScalarSensor::Reset();
    accumulatedYawDrift = Scalar(0);
}

void IMU::setRange(Vector3 angularVelocityMax)
{
    channels[3].rangeMin = -angularVelocityMax.x();
    channels[4].rangeMin = -angularVelocityMax.y();
    channels[5].rangeMin = -angularVelocityMax.z();
    channels[3].rangeMax = angularVelocityMax.x();
    channels[4].rangeMax = angularVelocityMax.y();
    channels[5].rangeMax = angularVelocityMax.z();
}
    
void IMU::setNoise(Vector3 angleStdDev, Vector3 angularVelocityStdDev, Scalar yawAngleDrift)
{
    channels[0].setStdDev(angleStdDev.x());
    channels[1].setStdDev(angleStdDev.y());
    channels[2].setStdDev(angleStdDev.z());
    channels[3].setStdDev(angularVelocityStdDev.x());
    channels[4].setStdDev(angularVelocityStdDev.y());
    channels[5].setStdDev(angularVelocityStdDev.z());
    yawDriftRate = yawAngleDrift;
}

ScalarSensorType IMU::getScalarSensorType()
{
    return ScalarSensorType::IMU;
}


}
