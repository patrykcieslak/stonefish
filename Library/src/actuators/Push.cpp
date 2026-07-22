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
//  Push.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/07/2023.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/Push.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"

namespace sf
{

Push::Push(const std::string& uniqueName, bool inverted) : LinkActuator(uniqueName)
{
    setpoint_ = Scalar(0);
    inv_ = inverted;
    setForceLimits(BT_LARGE_FLOAT, BT_LARGE_FLOAT); // No limits
}

LinkActuatorType Push::getLinkActuatorType() const
{
    return LinkActuatorType::PUSH;
}

void Push::setForceLimits(Scalar positive, Scalar negative)
{
    limits_.first = btClamped(positive, Scalar(0), Scalar(BT_LARGE_FLOAT));
    limits_.second = btClamped(btFabs(negative), Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Push::setForce(Scalar f)
{
    setpoint_ = btClamped(f, -limits_.second, limits_.first);
    ResetWatchdog();
}

Scalar Push::getForce() const
{
    return setpoint_;
}

void Push::Update(Scalar dt)
{    
    Actuator::Update(dt);

    if(attach_ != nullptr)
    {
        //Get transforms
        Transform solidTrans = attach_->getCGTransform();
        Transform pushTrans = attach_->getOTransform() * o2a_;
        
        Scalar force = inv_ ? -setpoint_ : setpoint_;
        
        Vector3 forceV(force, 0, 0);
        attach_->ApplyCentralForce(pushTrans.getBasis() * forceV);
        attach_->ApplyTorque((pushTrans.getOrigin() - solidTrans.getOrigin()).cross(pushTrans.getBasis() * forceV));    
    }
}

std::vector<Renderable> Push::Render()
{
    Transform pushTrans = Transform::getIdentity();
    if(attach_ != nullptr)
        pushTrans = attach_->getOTransform() * o2a_;
    else
        LinkActuator::Render();
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.model = glMatrixFromTransform(pushTrans);  
    item.type = RenderableType::ACTUATOR_LINES;
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    points->push_back(glm::vec3(0,0,0));
    points->push_back(glm::vec3(0.1f*(inv_ ? -setpoint_ : setpoint_),0,0));
    items.push_back(item);
    
    return items;
}

void Push::WatchdogTimeout()
{
    setForce(Scalar(0));
}

// Statics

ConstructInfo Push::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Specs
    node.optional = true;
    node.attributes.insert({"inverted", {ConstructInfoValueType::BOOL, true}});
    info.nodes.insert({"specs", node});

    // Limits
    node.optional = true;
    node.attributes.clear();
    node.attributes.insert({"max_positive_force", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"max_negative_force", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"limits", node});

    return info;
}

std::unique_ptr<Push> Push::Construct(const std::string& uniqueName, ConstructInfo& info)
{
    bool inverted = false;
    ConstructInfoValue& value = info.nodes.at("specs").attributes.at("inverted");
    if (value.valid)
        inverted = std::get<bool>(value.value);

    Scalar maxPositiveForce (BT_LARGE_FLOAT);
    Scalar maxNegativeForce (BT_LARGE_FLOAT);
    value = info.nodes.at("limits").attributes.at("max_positive_force");
    if (value.valid)
        maxPositiveForce = std::get<Scalar>(value.value);
    value = info.nodes.at("limits").attributes.at("max_negative_force");
    if (value.valid)
        maxNegativeForce = std::get<Scalar>(value.value);

    std::unique_ptr<Push> actuator = std::make_unique<Push>(uniqueName, inverted);
    actuator->setForceLimits(maxPositiveForce, maxNegativeForce);

    return actuator;
}

REGISTER_ACTUATOR("push", Push)
    
}
