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
//  Torque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Torque.h"

#include "core/DeviceFactory.h"
#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "sensors/Sample.h"

namespace sf
{

Torque::Torque(const std::string& uniqueName, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    channels_.push_back(SensorChannel("Torque", QuantityType::TORQUE));
}

void Torque::InternalUpdate(Scalar dt)
{
    //Vector3 force, torque;
    //fe->getJointFeedback(jId, force, torque);
    //Vector3 axis = fe->getJointAxis(jId);
    //Scalar tau = torque.dot(axis);
    if(fe_ != nullptr)
    {
        Scalar tau = fe_->getMotorForceTorque(jId_);
        AddSampleToHistory(std::make_unique<Sample>(std::vector<Scalar>({tau})));
    }
}

void Torque::setRange(Scalar torqueMax)
{
    channels_[0].rangeMin = -btClamped(torqueMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[0].rangeMax = btClamped(torqueMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Torque::setNoise(Scalar torqueStdDev)
{
    channels_[0].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Torque::getScalarSensorType() const
{
    return ScalarSensorType::TORQUE;
}

// Statics

ConstructInfo Torque::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // History
    node.optional = true;
    node.attributes.insert({"samples", {ConstructInfoValueType::INT, false}});
    info.nodes.insert({"history", node});

    // Range and noise
    node.attributes.clear(); // Clear temporary
    node.optional = true;
    node.attributes.insert({"torque", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"range", node});
    info.nodes.insert({"noise", node});
    
    return info;
}

std::unique_ptr<Torque> Torque::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Create sensor
    std::unique_ptr<Torque> sensor = std::make_unique<Torque>(uniqueName, frequency, history);    

    // Range (optional)
    value = info.nodes.at("range").attributes.at("torque");
    if (value.valid)
        sensor->setRange(std::get<Scalar>(value.value));

    // Noise (optional)
    value = info.nodes.at("noise").attributes.at("torque");
    if (value.valid)
        sensor->setNoise(std::get<Scalar>(value.value));

    return sensor;
}

REGISTER_SENSOR("torque", Torque)

}
