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
//  Compass.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Compass.h"

#include "sensors/Sample.h"
#include "core/DeviceFactory.h"

namespace sf
{

Compass::Compass(const std::string& uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels_.push_back(SensorChannel("Heading", QuantityType::ANGLE));
}

void Compass::InternalUpdate(Scalar dt)
{
    // Get angles
    Scalar yaw, pitch, roll;
    getSensorFrame().getBasis().getEulerYPR(yaw, pitch, roll);
    
    // Record sample
    AddSampleToHistory(std::make_unique<Sample>(std::vector<Scalar>({yaw})));
}

void Compass::setNoise(Scalar headingStdDev)
{
    channels_[0].setStdDev(btClamped(headingStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Compass::getScalarSensorType() const
{
    return ScalarSensorType::COMPASS;
}


ConstructInfo Compass::getConstructInfo()
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
    node.attributes.insert({"heading", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"noise", node});
    
    return info;
}

std::unique_ptr<Compass> Compass::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Create sensor
    std::unique_ptr<Compass> sensor = std::make_unique<Compass>(uniqueName, frequency, history);    

    // Noise (optional)
    value = info.nodes.at("noise").attributes.at("heading");
    if (value.valid)
        sensor->setNoise(std::get<Scalar>(value.value));

    return sensor;
}

REGISTER_SENSOR("compass", Compass)

}
