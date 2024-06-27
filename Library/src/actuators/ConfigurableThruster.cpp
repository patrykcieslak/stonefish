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
//  ConfigurableThruster.cpp
//  Stonefish
//
//  Created by Roger Pi on 03/06/2024
//  Copyright (c) 2024 Roger Pi. All rights reserved.
//

#include "actuators/ConfigurableThruster.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

// clamp implementation, to be removed when C++17 is available
template <class T>
const T& clamp(const T& v, const T& lo, const T& hi)
{
  return std::min(hi, std::max(lo, v));
}

namespace sf
{

ConfigurableThruster::ConfigurableThruster(std::string uniqueName, SolidEntity* propeller,     //
                                           std::shared_ptr<td::RotorDynamics> rotorDynamics,   //
                                           std::shared_ptr<td::ThrustModel> thrustConversion,  //
                                           bool rightHand, bool inverted)
  : LinkActuator(uniqueName)
  , RH_(rightHand)
  , inv_(inverted)
  , theta_(Scalar(0))
  , omega_(Scalar(0))
  , thrust_(Scalar(0))
  , torque_(Scalar(0))
  , setpoint_(Scalar(0))
  , rotorModel_(rotorDynamics)
  , thrustModel_(thrustConversion)
{
  setVelocityLimits(1, -1);  // No limits

  prop_ = propeller;
  prop_->BuildGraphicalObject();
}

ConfigurableThruster::~ConfigurableThruster()
{
  if (prop_ != nullptr)
    delete prop_;
}

ActuatorType ConfigurableThruster::getType() const
{
  return ActuatorType::CONFIGURABLE_THRUSTER;
}

void ConfigurableThruster::setSetpoint(Scalar s)
{
  setpoint_ = s;

  if (limits_.second > limits_.first)  // Limitted
    setpoint_ = clamp(setpoint_, limits_.first, limits_.second);

  if (inv_)
  {
    setpoint_ *= Scalar(-1);
  }

  ResetWatchdog();
}

void ConfigurableThruster::setVelocityLimits(Scalar lower, Scalar upper)
{
  limits_.first = lower;
  limits_.second = upper;
}

Scalar ConfigurableThruster::getSetpoint() const
{
  return inv_ ? -setpoint_ : setpoint_;
}

Scalar ConfigurableThruster::getAngle() const
{
  return theta_;
}

Scalar ConfigurableThruster::getOmega() const
{
  return omega_;
}

Scalar ConfigurableThruster::getThrust() const
{
  return thrust_;
}

Scalar ConfigurableThruster::getTorque() const
{
  return torque_;
}

void ConfigurableThruster::Update(Scalar dt)
{
  Actuator::Update(dt);

  if (attach == nullptr)
    return;  // No attachment, no action

  // Update rotation & angular velocity
  omega_ = rotorModel_->f(rotorModel_->getLastTime() + dt, setpoint_);
  omega_ = RH_ ? omega_ : -omega_;
  
  theta_ += omega_ * dt;  // Just for animation

  // Update Thrust
  thrust_ = thrustModel_->f(omega_);
  torque_ = Scalar(0);  // @TODO

  // Get transforms
  Transform solidTrans = attach->getCGTransform();
  Transform thrustTrans = attach->getOTransform() * o2a;

  // Calculate thrust
  Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
  if (ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
  {
    // Apply forces and torques
    Vector3 thrustV(thrust_, 0, 0);
    Vector3 torqueV(torque_, 0, 0);
    attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
    attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
    attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
  }
  else
  {
    thrust_ = Scalar(0);
    torque_ = Scalar(0);
  }
}

std::vector<Renderable> ConfigurableThruster::Render()
{
  Transform thrustTrans = Transform::getIdentity();
  if (attach != nullptr)
    thrustTrans = attach->getOTransform() * o2a;
  else
    LinkActuator::Render();

  // Rotate propeller
  thrustTrans *= Transform(Quaternion(0, 0, theta_), Vector3(0, 0, 0));

  // Add renderable
  std::vector<Renderable> items(0);
  Renderable item;
  item.type = RenderableType::SOLID;
  item.materialName = prop_->getMaterial().name;
  item.objectId = prop_->getGraphicalObject();
  item.lookId = dm == DisplayMode::GRAPHICAL ? prop_->getLook() : -1;
  item.model = glMatrixFromTransform(thrustTrans);
  items.push_back(item);

  item.type = RenderableType::ACTUATOR_LINES;
  item.points.push_back(glm::vec3(0, 0, 0));
  item.points.push_back(glm::vec3(0.1f * thrust_, 0, 0));
  items.push_back(item);

  return items;
}

void ConfigurableThruster::WatchdogTimeout()
{
  setSetpoint(Scalar(0));
}

}  // namespace sf
