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
//  Modified by Patryk Cieslak on 30/06/2024
//  Copyright (c) 2024 Roger Pi and Patryk Cieslak. All rights reserved.
//

#include "actuators/Thruster.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

Thruster::Thruster(std::string uniqueName, SolidEntity* propeller,
                                            std::shared_ptr<RotorDynamics> rotorDynamics,
                                            std::shared_ptr<ThrustModel> thrustConversion,
                                            Scalar diameter, bool rightHand, Scalar maxSetpoint,
                                            bool invertedSetpoint, bool normalizedSetpoint)
    : LinkActuator(uniqueName), RH(rightHand), D(diameter),
      theta(Scalar(0)), omega(Scalar(0)), thrust(Scalar(0)), torque(Scalar(0)),
      setpoint(Scalar(0)), setpointLimit(maxSetpoint), inv(invertedSetpoint), normalized(normalizedSetpoint),
      rotorModel(rotorDynamics), thrustModel(thrustConversion)
{
    setSetpointLimit(maxSetpoint);
    prop = propeller;
    prop->BuildGraphicalObject();
}

Thruster::~Thruster()
{
    if (prop != nullptr)
        delete prop;
}

ActuatorType Thruster::getType() const
{
    return ActuatorType::THRUSTER;
}

void Thruster::setSetpoint(Scalar s)
{
    if (normalized)
        setpoint = btClamped(s, Scalar(-1), Scalar(1)) * setpointLimit;
    else
        setpoint = btClamped(s, -setpointLimit, setpointLimit);
    if (inv) setpoint *= Scalar(-1);
    ResetWatchdog();
}

void Thruster::setSetpointLimit(Scalar limit)
{
    setpointLimit = btFabs(limit);
    rotorModel->setOutputLimit(setpointLimit*2); // Protect against uncontrolled behavior
}

Scalar Thruster::getSetpointLimit()
{
    return setpointLimit;
}

Scalar Thruster::getSetpoint() const
{
    return inv ? -setpoint : setpoint;
}

Scalar Thruster::getAngle() const
{
    return theta;
}

Scalar Thruster::getOmega() const
{
    return omega;
}

Scalar Thruster::getThrust() const
{
    return thrust;
}

Scalar Thruster::getTorque() const
{
    return torque;
}

bool Thruster::isPropellerRight() const
{
    return RH;
}

Scalar Thruster::getPropellerDiameter() const
{
    return D;
}

void Thruster::Update(Scalar dt)
{
    Actuator::Update(dt);

    if (attach == nullptr)
        return; // No attachment, no action

    // Update rotation & angular velocity
    if (rotorModel->getType() == RotorDynamicsType::MECHANICAL_PI)
        std::static_pointer_cast<MechanicalPI>(rotorModel)->setDampingTorque(btFabs(torque));
    omega = rotorModel->Update(dt, setpoint);
    theta += omega * dt; // Just for animation

    // Check if thruster is sumberged and compute thrust model
    Transform solidTrans = attach->getCGTransform();
    Transform thrustTrans = attach->getOTransform() * o2a;
    Ocean *ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();

    if (ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
    {
        // Update Thrust
        if (thrustModel->getType() == ThrustModelType::FD)
        {
            Vector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
            Vector3 velocity = attach->getLinearVelocityInLocalPoint(relPos);
            Scalar u = -thrustTrans.getBasis().getColumn(0).dot(ocn->GetFluidVelocity(thrustTrans.getOrigin()) - velocity);
            std::static_pointer_cast<FDThrust>(thrustModel)->setIncomingFluidVelocity(u);
        }
        std::pair<Scalar, Scalar> out = thrustModel->Update(omega);
        thrust = out.first;
        torque = out.second;

        // Account for handedness of the propeller
        if (!RH && thrustModel->getType() != ThrustModelType::FD)
            thrust = -thrust;
    
        // Apply forces and torques
        Vector3 thrustV(thrust, 0, 0);
        Vector3 torqueV(torque, 0, 0);
        attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
        attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
        attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
    }
    else
    {
        thrust = Scalar(0);
        torque = Scalar(0);
    }
}

std::vector<Renderable> Thruster::Render()
{
    Transform thrustTrans = Transform::getIdentity();
    if (attach != nullptr)
        thrustTrans = attach->getOTransform() * o2a;
    else
        LinkActuator::Render();

    // Rotate propeller
    thrustTrans *= Transform(Quaternion(0, 0, theta), Vector3(0, 0, 0));

    // Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = prop->getMaterial().name;
    item.objectId = prop->getGraphicalObject();
    item.lookId = dm == DisplayMode::GRAPHICAL ? prop->getLook() : -1;
    item.model = glMatrixFromTransform(thrustTrans);
    items.push_back(item);

    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0, 0, 0));
    item.points.push_back(glm::vec3(0.1f * thrust, 0, 0));
    items.push_back(item);

    return items;
}

void Thruster::WatchdogTimeout()
{
    setSetpoint(Scalar(0));
}

} // namespace sf
