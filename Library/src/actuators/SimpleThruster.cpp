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
//  Thruster.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 07/04/2024.
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/SimpleThruster.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

SimpleThruster::SimpleThruster(const std::string& uniqueName, std::unique_ptr<SolidEntity> propeller, bool rightHand, bool inverted) : LinkActuator(uniqueName)
{
    RH_ = rightHand;
    inv_ = inverted;
    theta_ = Scalar(0);
    thrust_ = Scalar(0);
    torque_ = Scalar(0);
    setThrustLimits(1, -1); // No limits
    
    propeller_ = std::move(propeller);
    propeller_->BuildGraphicalObject();
}

ActuatorType SimpleThruster::getType() const
{
    return ActuatorType::SIMPLE_THRUSTER;
}

void SimpleThruster::setSetpoint(Scalar _thrust, Scalar _torque)
{
    if(limits_.second > limits_.first) // Limitted
        sThrust_ = btClamped(_thrust, limits_.first, limits_.second);
    else // No limits
        sThrust_ = _thrust;
    
    sTorque_ = _torque;

    if(inv_)
    { 
        sThrust_ *= Scalar(-1);
        sTorque_ *= Scalar(-1);
    }

    ResetWatchdog();
}

void SimpleThruster::setThrustLimits(Scalar lower, Scalar upper)
{
    limits_.first = lower;
    limits_.second = upper;
}

Scalar SimpleThruster::getAngle() const
{
    return theta_;
}

Scalar SimpleThruster::getThrustSetpoint() const
{
    return sThrust_;
}

Scalar SimpleThruster::getThrust() const
{
    return thrust_;
}

Scalar SimpleThruster::getTorque() const
{
    return torque_;
}

void SimpleThruster::Update(Scalar dt)
{
    Actuator::Update(dt);

    if(attach_ != nullptr)
    {
        //Update thruster rotation (visualization only)
        Scalar omega(0);
        if(!btFuzzyZero(sThrust_))
        {
            omega = sThrust_ > Scalar(0) ? Scalar(2.0*M_PI) : Scalar(-2.0*M_PI);
            omega = RH_ ? omega : -omega; 
        }
        theta_ += omega * dt; //Just for animation
    
        //Get transforms
        Transform solidTrans = attach_->getCGTransform();
        Transform thrustTrans = attach_->getOTransform() * o2a_;
        
        //Calculate thrust
        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
        if(ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
        {
            thrust_ = sThrust_;
            torque_ = sTorque_;

            //Apply forces and torques
            Vector3 thrustV(thrust_, 0, 0);
            Vector3 torqueV(torque_, 0, 0);
            attach_->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach_->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach_->ApplyTorque(thrustTrans.getBasis() * torqueV);
        }
        else
        {
            thrust_ = Scalar(0);
            torque_ = Scalar(0);
        }
    }
}

std::vector<Renderable> SimpleThruster::Render()
{
    Transform thrustTrans = Transform::getIdentity();
    if(attach_ != nullptr)
        thrustTrans = attach_->getOTransform() * o2a_;
    else
        LinkActuator::Render();
    
    //Rotate propeller
    thrustTrans *= Transform(Quaternion(0, 0, theta_), Vector3(0,0,0));
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = propeller_->getMaterial().name;
    item.objectId = propeller_->getGraphicalObject();
    item.lookId = dm_ == DisplayMode::GRAPHICAL ? propeller_->getLook() : -1;
	item.model = glMatrixFromTransform(thrustTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    points->push_back(glm::vec3(0,0,0));
    points->push_back(glm::vec3(0.1f*thrust_,0,0));
    items.push_back(item);
    
    return items;
}

void SimpleThruster::WatchdogTimeout()
{
    setSetpoint(Scalar(0), Scalar(0));
}
    
}
