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
//  Created by Roger Pi on 03/06/2024
//  Modified by Patryk Cieslak on 06/07/2026
//  Copyright (c) 2024-2026 Roger Pi and Patryk Cieslak. All rights reserved.
//

#include "actuators/Thruster.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

Thruster::Thruster(const std::string& uniqueName, std::unique_ptr<SolidEntity> propeller,
                                            std::unique_ptr<RotorDynamics> rotorDynamics,
                                            std::unique_ptr<ThrustModel> thrustModel,
                                            Scalar diameter, bool rightHand, Scalar maxSetpoint,
                                            bool invertedSetpoint, bool normalizedSetpoint)
    : LinkActuator(uniqueName), propeller_(std::move(propeller)), RH_(rightHand), D_(diameter),
      theta_(Scalar(0)), omega_(Scalar(0)), thrust_(Scalar(0)), torque_(Scalar(0)),
      setpoint_(Scalar(0)), setpointLimit_(maxSetpoint), inv_(invertedSetpoint), normalized_(normalizedSetpoint),
      rotorModel_(std::move(rotorDynamics)), thrustModel_(std::move(thrustModel))
{
    setSetpointLimit(maxSetpoint);
    propeller_->BuildGraphicalObject();
}

ActuatorType Thruster::getType() const
{
    return ActuatorType::THRUSTER;
}

void Thruster::setSetpoint(Scalar s)
{
    if (normalized_)
        setpoint_ = btClamped(s, Scalar(-1), Scalar(1)) * setpointLimit_;
    else
        setpoint_ = btClamped(s, -setpointLimit_, setpointLimit_);
    if (inv_) setpoint_ *= Scalar(-1);
    ResetWatchdog();
}

void Thruster::setSetpointLimit(Scalar limit)
{
    setpointLimit_ = btFabs(limit);
    rotorModel_->setOutputLimit(setpointLimit_*2); // Protect against uncontrolled behavior
}

Scalar Thruster::getSetpointLimit()
{
    return setpointLimit_;
}

Scalar Thruster::getSetpoint() const
{
    return inv_ ? -setpoint_ : setpoint_;
}

Scalar Thruster::getAngle() const
{
    return theta_;
}

Scalar Thruster::getOmega() const
{
    return omega_;
}

Scalar Thruster::getThrust() const
{
    return thrust_;
}

Scalar Thruster::getTorque() const
{
    return torque_;
}

bool Thruster::isPropellerRight() const
{
    return RH_;
}

Scalar Thruster::getPropellerDiameter() const
{
    return D_;
}

void Thruster::Update(Scalar dt)
{
    Actuator::Update(dt);

    if (attach_ == nullptr)
        return; // No attachment, no action

    // Update rotation & angular velocity
    if (rotorModel_->getType() == RotorDynamicsType::MECHANICAL_PI)
        static_cast<MechanicalPI*>(rotorModel_.get())->setDampingTorque(btFabs(torque_));
    omega_ = rotorModel_->Update(dt, setpoint_);
    theta_ += omega_ * dt; // Just for animation

    // Check if thruster is sumberged and compute thrust model
    Transform solidTrans = attach_->getCGTransform();
    Transform thrustTrans = attach_->getOTransform() * o2a_;
    Ocean *ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();

    if (ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
    {
        // Update Thrust
        if (thrustModel_->getType() == ThrustModelType::FD)
        {
            Vector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
            Vector3 velocity = attach_->getLinearVelocityInLocalPoint(relPos);
            Scalar u = -thrustTrans.getBasis().getColumn(0).dot(ocn->GetFluidVelocity(thrustTrans.getOrigin()) - velocity);
            static_cast<FDThrust*>(thrustModel_.get())->setIncomingFluidVelocity(u);
        }
        std::pair<Scalar, Scalar> out = thrustModel_->Update(omega_);
        thrust_ = out.first;
        torque_ = out.second;

        // Account for handedness of the propeller
        if (!RH_ && thrustModel_->getType() != ThrustModelType::FD)
            thrust_ = -thrust_;
    
        // Apply forces and torques
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

std::vector<Renderable> Thruster::Render()
{
    Transform thrustTrans = Transform::getIdentity();
    if (attach_ != nullptr)
        thrustTrans = attach_->getOTransform() * o2a_;
    else
        LinkActuator::Render();

    // Rotate propeller
    thrustTrans *= Transform(Quaternion(0, 0, theta_), Vector3(0, 0, 0));

    // Add renderable
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
    points->push_back(glm::vec3(0, 0, 0));
    points->push_back(glm::vec3(0.1f * thrust_, 0, 0));
    items.push_back(item);

    return items;
}

void Thruster::WatchdogTimeout()
{
    setSetpoint(Scalar(0));
}

} // namespace sf
