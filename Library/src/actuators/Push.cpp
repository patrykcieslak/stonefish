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

namespace sf
{

Push::Push(const std::string& uniqueName, bool inverted) : LinkActuator(uniqueName)
{
    setpoint_ = Scalar(0);
    inv_ = inverted;
    setForceLimits(1, -1); // No limits
}

ActuatorType Push::getType() const
{
    return ActuatorType::PUSH;
}

void Push::setForceLimits(Scalar lower, Scalar upper)
{
    limits_.first = lower;
    limits_.second = upper;
}

void Push::setForce(Scalar f)
{
    if(limits_.second > limits_.first) // Limitted
        setpoint_ = f < limits_.first ? limits_.first : (f > limits_.second ? limits_.second : f);
    else
        setpoint_ = f;
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
    
}
