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
//  LinkActuatorPlugin.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2026.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#include "LinkActuatorPlugin.h"

#include <entities/SolidEntity.h>

namespace sf
{

LinkActuatorPlugin::LinkActuatorPlugin(std::string_view uniqueName, Scalar amplitude, Scalar frequency)
    : LinkActuator(uniqueName), amplitude_(btFabs(amplitude)), frequency_(btFabs(frequency)), time_(0.0)
{
}

void LinkActuatorPlugin::Update(Scalar dt)
{
    Actuator::Update(dt);

    time_ += dt;
    Scalar force = amplitude_ * btSin(Scalar(2.0 * M_PI) * frequency_ * time_);
    
    if(attach_ != nullptr)
    {
        //Get transforms
        Transform solidTrans = attach_->GetCgTransform();
        Transform actuatorTrans = attach_->GetOTransform() * o2a_;
        
        attach_->ApplyCentralForce(actuatorTrans.getBasis() * Vector3(force, 0, 0));
        attach_->ApplyTorque((actuatorTrans.getOrigin() - solidTrans.getOrigin()).cross(actuatorTrans.getBasis() * Vector3(force, 0, 0)));
    }
}

LinkActuatorType LinkActuatorPlugin::GetLinkActuatorType() const
{
    return LinkActuatorType::PLUGIN;
}

ConstructInfo LinkActuatorPlugin::GetConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;

    // Specs
    node.optional = false;
    node.attributes.insert({"amplitude", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"frequency", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"specs", node});

    return info;
}

LinkActuatorPlugin* LinkActuatorPlugin::Construct(std::string_view uniqueName, ConstructInfo* info)
{
    // Required
    Scalar amplitude = std::get<Scalar>(info->nodes.at("specs").attributes.at("amplitude").value);
    Scalar frequency = std::get<Scalar>(info->nodes.at("specs").attributes.at("frequency").value);

    return new LinkActuatorPlugin(uniqueName, amplitude, frequency);
}

extern "C" 
{
    Actuator* create(const char* uniqueName, ConstructInfo* info)
    {
        return LinkActuatorPlugin::Construct(uniqueName, info);
    }

    void destroy(Actuator* actuator) 
    {
        delete actuator;
    }

    ConstructInfo getConstructInfo()
    {
        return LinkActuatorPlugin::GetConstructInfo();
    }
}

}