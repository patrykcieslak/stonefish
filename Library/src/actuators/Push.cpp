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
//  Copyright (c) 2023-2024 Patryk Cieslak. All rights reserved.
//

#include "actuators/Push.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Push::Push(std::string uniqueName, bool inverted, bool onlyWorksSubmerged) : LinkActuator(uniqueName)
{
    setpoint = Scalar(0);
    inv = inverted;
    underwater = onlyWorksSubmerged;
    setForceLimits(1, -1); // No limits
}

ActuatorType Push::getType() const
{
    return ActuatorType::PUSH;
}

void Push::setForceLimits(double lower, double upper)
{
    limits.first = lower;
    limits.second = upper;
}

void Push::setForce(Scalar f)
{
    if(limits.second > limits.first) // Limitted
        setpoint = f < limits.first ? limits.first : (f > limits.second ? limits.second : f);
    else
        setpoint = f;
    ResetWatchdog();
}

Scalar Push::getForce() const
{
    return setpoint;
}

void Push::Update(Scalar dt)
{    
    Actuator::Update(dt);

    if(attach != nullptr)
    {
        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        Transform pushTrans = attach->getOTransform() * o2a;
        
        Scalar force(0);
        if(underwater)
        {
            Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
            if(ocn != nullptr && ocn->IsInsideFluid(pushTrans.getOrigin()))
                force = inv ? -setpoint : setpoint;
        }
        else
            force = inv ? -setpoint : setpoint;
        
        Vector3 forceV(force, 0, 0);
        attach->ApplyCentralForce(pushTrans.getBasis() * forceV);
        attach->ApplyTorque((pushTrans.getOrigin() - solidTrans.getOrigin()).cross(pushTrans.getBasis() * forceV));    
    }
}

std::vector<Renderable> Push::Render()
{
    Transform pushTrans = Transform::getIdentity();
    if(attach != nullptr)
        pushTrans = attach->getOTransform() * o2a;
    else
        LinkActuator::Render();
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.model = glMatrixFromTransform(pushTrans);  
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0.1f*(inv ? -setpoint : setpoint),0,0));
    items.push_back(item);
    
    return items;
}

void Push::WatchdogTimeout()
{
    setForce(Scalar(0));
}
    
}
