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
//  Odometry.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Odometry.h"

#include "core/DeviceFactory.h"
#include "entities/MovingEntity.h"
#include "sensors/Sample.h"

namespace sf
{

Odometry::Odometry(const std::string& uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels_.push_back(SensorChannel("Position X", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Position Y", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Position Z", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Velocity X", QuantityType::VELOCITY));
    channels_.push_back(SensorChannel("Velocity Y", QuantityType::VELOCITY));
    channels_.push_back(SensorChannel("Velocity Z", QuantityType::VELOCITY));
    channels_.push_back(SensorChannel("Orientation X", QuantityType::UNITLESS));
    channels_.push_back(SensorChannel("Orientation Y", QuantityType::UNITLESS));
    channels_.push_back(SensorChannel("Orientation Z", QuantityType::UNITLESS));
    channels_.push_back(SensorChannel("Orientation W", QuantityType::UNITLESS));
    channels_.push_back(SensorChannel("Angular velocity X", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Angular velocity Y", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Angular velocity Z", QuantityType::ANGULAR_VELOCITY));
    ornStdDev_ = Scalar(0);
    ornNoise_ = std::normal_distribution<Scalar>(Scalar(0), ornStdDev_);
}

void Odometry::InternalUpdate(Scalar dt)
{
    //Calculate transformation from global to imu frame
    Transform odomTrans = getSensorFrame();
    
    Vector3 pos = odomTrans.getOrigin();
    Vector3 v = odomTrans.getBasis().inverse() * attach_->getLinearVelocityInLocalPoint(odomTrans.getOrigin() - attach_->getCGTransform().getOrigin());
    
    Quaternion orn = odomTrans.getRotation();
    Scalar angle = orn.getAngle() + ornNoise_(randomGenerator);
    orn = Quaternion(orn.getAxis(), angle);

    Vector3 av = odomTrans.getBasis().inverse() * attach_->getAngularVelocity();
    
    //Record sample
    AddSampleToHistory(std::make_unique<Sample>(
        std::vector<Scalar>({pos.x(), pos.y(), pos.z(), v.x(), v.y(), v.z(), orn.x(), orn.y(), orn.z(), orn.w(), av.x(), av.y(), av.z()})
    ));
}
   
void Odometry::setNoise(Scalar positionStdDev, Scalar velocityStdDev, Scalar angleStdDev, Scalar angularVelocityStdDev)
{
    channels_[0].setStdDev(btClamped(positionStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[1].setStdDev(btClamped(positionStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[2].setStdDev(btClamped(positionStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[3].setStdDev(btClamped(velocityStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[4].setStdDev(btClamped(velocityStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[5].setStdDev(btClamped(velocityStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[10].setStdDev(btClamped(angularVelocityStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[11].setStdDev(btClamped(angularVelocityStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[12].setStdDev(btClamped(angularVelocityStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    ornStdDev_ = btClamped(angleStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT));
    ornNoise_ = std::normal_distribution<Scalar>(Scalar(0), ornStdDev_);
}

ScalarSensorType Odometry::getScalarSensorType() const
{
    return ScalarSensorType::ODOM;
}

// Statics

ConstructInfo Odometry::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // History
    node.optional = true;
    node.attributes.insert({"samples", {ConstructInfoValueType::INT, false}});
    info.nodes.insert({"history", node});

    // Noise
    node.attributes.clear(); // Clear temporary
    node.optional = true;
    node.attributes.insert({"position", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"velocity", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"angle", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"angular_velocity", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"noise", node});
    
    return info;
}

std::unique_ptr<Odometry> Odometry::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Create sensor
    std::unique_ptr<Odometry> sensor = std::make_unique<Odometry>(uniqueName, frequency, history);    

    // Noise (optional)
    Scalar position (0.);
    Scalar velocity (0.);
    Scalar angle (0.);
    Scalar angularVelocity (0.);

    value = info.nodes.at("noise").attributes.at("position");
    if (value.valid)
        position = std::get<Scalar>(value.value);
    
    value = info.nodes.at("noise").attributes.at("velocity");
    if (value.valid)
        velocity = std::get<Scalar>(value.value);

    value = info.nodes.at("noise").attributes.at("angle");
    if (value.valid)
        angle = std::get<Scalar>(value.value);

    value = info.nodes.at("noise").attributes.at("angular_velocity");
    if (value.valid)
        angularVelocity = std::get<Scalar>(value.value);

    sensor->setNoise(position, velocity, angle, angularVelocity);

    return sensor;
}

REGISTER_SENSOR("odometry", Odometry)

}
