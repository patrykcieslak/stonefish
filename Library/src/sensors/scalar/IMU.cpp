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
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/IMU.h"

#include "entities/MovingEntity.h"
#include "sensors/Sample.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

IMU::IMU(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Roll", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Pitch", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Yaw", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Angular velocity X", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Linear acceleration X", QuantityType::ACCELERATION));
    channels.push_back(SensorChannel("Linear acceleration Y", QuantityType::ACCELERATION));
    channels.push_back(SensorChannel("Linear acceleration Z", QuantityType::ACCELERATION));
    
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

    //get acceleration
    Vector3 R = imuTrans.getOrigin() - attach->getCGTransform().getOrigin();
    Vector3 la = imuTrans.getBasis().inverse() * (
                   attach->getLinearAcceleration() 
                   + attach->getAngularAcceleration().cross(R)
                   + attach->getAngularVelocity().cross(attach->getAngularVelocity().cross(R))
                   - SimulationApp::getApp()->getSimulationManager()->getGravity() // Negative to get readings like in actual sensor
                );
    
    //record sample
    Scalar values[9] = {roll, pitch, yaw, av.x(), av.y(), av.z(), la.x(), la.y(), la.z()};
    Sample s(9, values);
    AddSampleToHistory(s);
}

void IMU::Reset()
{
    ScalarSensor::Reset();
    accumulatedYawDrift = Scalar(0);
}

void IMU::setRange(Vector3 angularVelocityMax, Vector3 linearAccelerationMax)
{
    channels[3].rangeMin = -btClamped(angularVelocityMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[4].rangeMin = -btClamped(angularVelocityMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[5].rangeMin = -btClamped(angularVelocityMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[3].rangeMax = btClamped(angularVelocityMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[4].rangeMax = btClamped(angularVelocityMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[5].rangeMax = btClamped(angularVelocityMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[6].rangeMin = -btClamped(linearAccelerationMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[7].rangeMin = -btClamped(linearAccelerationMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[8].rangeMin = -btClamped(linearAccelerationMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[6].rangeMax = btClamped(linearAccelerationMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[7].rangeMax = btClamped(linearAccelerationMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[8].rangeMax = btClamped(linearAccelerationMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void IMU::setNoise(Vector3 angleStdDev, Vector3 angularVelocityStdDev, Scalar yawAngleDrift, Vector3 linearAccelerationStdDev)
{
    channels[0].setStdDev(btClamped(angleStdDev.x(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[1].setStdDev(btClamped(angleStdDev.y(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[2].setStdDev(btClamped(angleStdDev.z(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[3].setStdDev(btClamped(angularVelocityStdDev.x(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[4].setStdDev(btClamped(angularVelocityStdDev.y(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[5].setStdDev(btClamped(angularVelocityStdDev.z(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[6].setStdDev(btClamped(linearAccelerationStdDev.x(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[7].setStdDev(btClamped(linearAccelerationStdDev.y(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[8].setStdDev(btClamped(linearAccelerationStdDev.z(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    yawDriftRate = yawAngleDrift;
}

ScalarSensorType IMU::getScalarSensorType()
{
    return ScalarSensorType::IMU;
}

}
