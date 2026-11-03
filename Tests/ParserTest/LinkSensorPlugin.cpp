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
//  LinkSensorPlugin.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/07/2026.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#include "LinkSensorPlugin.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

LinkSensorPlugin::LinkSensorPlugin(std::string_view uniqueName, bool globalFrame, Scalar frequency, int historyLength)
        : LinkSensor(uniqueName, frequency, historyLength), globalFrame_(globalFrame)
{
    channels_.push_back(SensorChannel("Fluid velocity X", QuantityType::VELOCITY));
    channels_.push_back(SensorChannel("Fluid velocity Y", QuantityType::VELOCITY));
    channels_.push_back(SensorChannel("Fluid velocity Z", QuantityType::VELOCITY));
}

ScalarSensorType LinkSensorPlugin::GetScalarSensorType() const
{ 
    return ScalarSensorType::PLUGIN; 
}

void LinkSensorPlugin::SetNoise(Vector3 fluidVelocityStdDev)
{
    channels_[0].SetStdDev(btClamped(fluidVelocityStdDev.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[1].SetStdDev(btClamped(fluidVelocityStdDev.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[2].SetStdDev(btClamped(fluidVelocityStdDev.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

void LinkSensorPlugin::InternalUpdate(Scalar dt)
{
    Vector3 data(0.0, 0.0, 0.0); //Fluid velocity [m/s]
    
    Ocean* liq = SimulationApp::GetApp()->GetSimulationManager()->GetOcean();
    if(liq != nullptr && liq->IsInsideFluid(GetSensorFrame().getOrigin()))
    {
        if(globalFrame_)
            data = liq->GetFluidVelocity(GetSensorFrame().getOrigin());
        else
            data = GetSensorFrame().getBasis().inverse() * liq->GetFluidVelocity(GetSensorFrame().getOrigin());
    }    
    
    //Record sample
    AddSampleToHistory(std::make_unique<Sample>(std::vector<Scalar>({data.getX(), data.getY(), data.getZ()})));
}

// Statics
ConstructInfo LinkSensorPlugin::GetConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // History
    node.optional = true;
    node.attributes.insert({"samples", {ConstructInfoValueType::INT, false}});
    info.nodes.insert({"history", node});

    // Specs
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"global_frame", {ConstructInfoValueType::BOOL, false}});
    info.nodes.insert({"specs", node});

    // Noise
    node.attributes.clear(); // Clear temporary
    node.optional = true;
    node.attributes.insert({"fluid_velocity", {ConstructInfoValueType::VECTOR3, false}});
    info.nodes.insert({"noise", node});
    
    return info;
}

LinkSensorPlugin* LinkSensorPlugin::Construct(std::string_view uniqueName, Scalar frequency, ConstructInfo* info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info->nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);

    // Specs
    bool globalFrame = std::get<bool>(info->nodes.at("specs").attributes.at("global_frame").value);
    
    // Create sensor
    LinkSensorPlugin* sensor = new LinkSensorPlugin(uniqueName, globalFrame, frequency, history);    

    // Noise (optional)
    value = info->nodes.at("noise").attributes.at("fluid_velocity");
    if (value.valid)
        sensor->SetNoise(std::get<Vector3>(value.value));

    return sensor;
}

extern "C" 
{
    Sensor* create(const char* uniqueName, Scalar frequency, ConstructInfo* info)
    {
        return LinkSensorPlugin::Construct(uniqueName, frequency, info);
    }

    void destroy(Sensor* sensor) 
    {
        delete sensor;
    }

    ConstructInfo getConstructInfo()
    {
        return LinkSensorPlugin::GetConstructInfo();
    }
}

}
