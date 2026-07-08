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
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Accelerometer.h"

#include "entities/MovingEntity.h"
#include "sensors/Sample.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Accelerometer::Accelerometer(const std::string& uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels_.push_back(SensorChannel("Linear Acceleration X", QuantityType::ACCELERATION));
    channels_.push_back(SensorChannel("Linear Acceleration Y", QuantityType::ACCELERATION));
    channels_.push_back(SensorChannel("Linear Acceleration Z", QuantityType::ACCELERATION));
}

void Accelerometer::InternalUpdate(Scalar dt)
{
    // Calculate transformation from global to imu frame
    Transform accTrans = getSensorFrame();
    
    // Get acceleration
    Vector3 R = accTrans.getOrigin() - attach_->getCGTransform().getOrigin();
    Vector3 la = accTrans.getBasis().inverse() * (
                                                attach_->getLinearAcceleration() 
                                                + attach_->getAngularAcceleration().cross(R)
                                                + attach_->getAngularVelocity().cross(attach_->getAngularVelocity().cross(R))
                                                - SimulationApp::getApp()->getSimulationManager()->getGravity()
                                                );
    
    // Record sample
    AddSampleToHistory(std::make_unique<Sample>(std::vector<Scalar>({la.x(), la.y(), la.z()})));
}

void Accelerometer::setRange(Vector3 linearAccelerationMax)
{
    channels_[0].rangeMin = -btClamped(linearAccelerationMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMin = -btClamped(linearAccelerationMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[2].rangeMin = -btClamped(linearAccelerationMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[0].rangeMax = btClamped(linearAccelerationMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMax = btClamped(linearAccelerationMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[2].rangeMax = btClamped(linearAccelerationMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void Accelerometer::setNoise(Vector3 linearAccelerationStdDev)
{
    channels_[0].setStdDev(btClamped(linearAccelerationStdDev.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[1].setStdDev(btClamped(linearAccelerationStdDev.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[2].setStdDev(btClamped(linearAccelerationStdDev.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Accelerometer::getScalarSensorType() const
{
    return ScalarSensorType::ACC;
}
    
}
