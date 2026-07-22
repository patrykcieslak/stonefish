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
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Gyroscope.h"

#include "core/DeviceFactory.h"
#include "entities/MovingEntity.h"
#include "sensors/Sample.h"

namespace sf
{

Gyroscope::Gyroscope(const std::string& uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels_.push_back(SensorChannel("Angular velocity X", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Angular velocity Y", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Angular velocity Z", QuantityType::ANGULAR_VELOCITY));
    bias_ = V0();
}

void Gyroscope::InternalUpdate(Scalar dt)
{
    //calculate transformation from global to gyro frame
    Matrix3 toGyroFrame = getSensorFrame().getBasis().inverse();
    
    //get angular velocity
    Vector3 omega = toGyroFrame * attach_->getAngularVelocity();

    //add bias error
    omega += bias_;

    //record sample
    AddSampleToHistory(std::make_unique<Sample>(std::vector<Scalar>({omega.x(), omega.y(), omega.z()})));
}

void Gyroscope::setRange(Vector3 angularVelocityMax)
{
    channels_[0].rangeMin = -btClamped(angularVelocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMin = -btClamped(angularVelocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[2].rangeMin = -btClamped(angularVelocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[0].rangeMax = btClamped(angularVelocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMax = btClamped(angularVelocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[2].rangeMax = btClamped(angularVelocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Gyroscope::setNoise(Vector3 angularVelocityStdDev, Vector3 angularVelocityBias)
{
    channels_[0].setStdDev(btClamped(angularVelocityStdDev.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[1].setStdDev(btClamped(angularVelocityStdDev.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[2].setStdDev(btClamped(angularVelocityStdDev.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    bias_ = angularVelocityBias;
}

ScalarSensorType Gyroscope::getScalarSensorType() const
{
    return ScalarSensorType::GYRO;
}

// Statics

ConstructInfo Gyroscope::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoValue value;
    ConstructInfoNode node;
    
    // History
    value.valueType = ConstructInfoValueType::INT;
    value.optional = false;
    node.optional = true;
    node.attributes.insert({"samples", value});
    info.nodes.insert({"history", node});

    // Range
    node.attributes.clear(); // Clear temporary

    value.valueType = ConstructInfoValueType::VECTOR3;
    value.optional = false;
    node.optional = true;
    node.attributes.insert({"angular_velocity", value});
    info.nodes.insert({"range", node});

    // Noise
    node.attributes.clear();

    value.valueType = ConstructInfoValueType::VECTOR3;
    value.optional = true;
    node.optional = true;
    node.attributes.insert({"angular_velocity", value});
    node.attributes.insert({"bias", value});
    info.nodes.insert({"noise", node});
    
    return info;
}

std::unique_ptr<Gyroscope> Gyroscope::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Create sensor
    std::unique_ptr<Gyroscope> sensor = std::make_unique<Gyroscope>(uniqueName, frequency, history);    

    // Range (optional)
    value = info.nodes.at("range").attributes.at("angular_velocity");
    if (value.valid)
        sensor->setRange(std::get<Vector3>(value.value));

    // Noise (optional)
    Vector3 noise = V0();
    Vector3 bias = V0();
    value = info.nodes.at("noise").attributes.at("angular_veloicty");
    if (value.valid)
        noise = std::get<Vector3>(value.value);
    value = info.nodes.at("noise").attributes.at("bias");
    if (value.valid)
        bias = std::get<Vector3>(value.value);
    sensor->setNoise(noise, bias);

    return sensor;
}

REGISTER_SENSOR("gyroscope", Gyroscope)

}
