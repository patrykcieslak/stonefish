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
//  Pressure.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Pressure.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"
#include "sensors/Sample.h"

namespace sf
{

Pressure::Pressure(const std::string& uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels_.push_back(SensorChannel("Pressure", QuantityType::PRESSURE));
}

void Pressure::InternalUpdate(Scalar dt)
{
    Scalar data(0.); //Gauge pressure //data(101325.); //Pa (1 atm)
    
    Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(liq != NULL)
        data += liq->GetPressure(getSensorFrame().getOrigin());
    
    //Record sample
    AddSampleToHistory(std::make_unique<Sample>(std::vector<Scalar>({data})));
}

void Pressure::setRange(Scalar max)
{
    channels_[0].rangeMin = Scalar(0);
    channels_[0].rangeMax = btClamped(max, Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void Pressure::setNoise(Scalar pressureStdDev)
{
    channels_[0].setStdDev(btClamped(pressureStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Pressure::getScalarSensorType() const
{
    return ScalarSensorType::PRESSURE;
}

// Statics

ConstructInfo Pressure::getConstructInfo()
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
    node.attributes.insert({"pressure", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"range", node});
    info.nodes.insert({"noise", node});
    
    return info;
}

std::unique_ptr<Pressure> Pressure::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Create sensor
    std::unique_ptr<Pressure> sensor = std::make_unique<Pressure>(uniqueName, frequency, history);    

    // Range (optional)
    value = info.nodes.at("range").attributes.at("pressure");
    if (value.valid)
        sensor->setRange(std::get<Scalar>(value.value));

    // Noise (optional)
    value = info.nodes.at("noise").attributes.at("pressure");
    if (value.valid)
        sensor->setNoise(std::get<Scalar>(value.value));

    return sensor;
}

REGISTER_SENSOR("pressure", Pressure)

}
