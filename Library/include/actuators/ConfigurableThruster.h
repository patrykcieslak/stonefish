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
//  ConfigurableThruster.h
//  Stonefish
//
//  Created by Roger Pi on 03/06/2024
//  Copyright (c) 2024 Roger Pi. All rights reserved.
//

#pragma once

#include "actuators/LinkActuator.h"
#include "ConfigurableThrusterModels.h"

namespace sf
{
//! A class representing a thruster.
class ConfigurableThruster : public LinkActuator
{
public:
  //! A constructor.
  /*!
   \param uniqueName a name for the thruster
   \param propeller a pointer to a rigid body representing the propeller
   \param diameter the diameter of the propeller [m]
   \param thrustCoeff the thrust coefficient (forward and backward)
   \param torqueCoeff the torque coefficient
   \param maxRPM the maximum rotational speed of the thruster [rpm]
   \param rightHand a flag to indicate if the propeller is right hand (clockwise rotation)
   \param inverted a flag to indicate if the setpoint is inverted (positive value results in backward force)
  */
  ConfigurableThruster(std::string uniqueName, SolidEntity* propeller,                      //
                 std::shared_ptr<rm::RotorDynamics> rotorDynamics,        //
                 std::shared_ptr<tm::ThrustModel> thrustConversion,  //
                 bool rightHand, bool inverted = false);

  //! A destructor.
  ~ConfigurableThruster();

  //! A method used to update the internal state of the thruster.
  /*!
   \param dt a time step of the simulation [s]
   */
  void Update(Scalar dt);

  //! A method implementing the rendering of the thruster.
  std::vector<Renderable> Render();

  //! A method setting the new value of the thruster speed setpoint.
  /*!
   \param s the desired speed of the thruster as fraction <0,1>
   */
  void setSetpoint(Scalar s);

  //! A method used to set the velocity limits.
  void setVelocityLimits(Scalar lower, Scalar upper);

  //! A method returning the current setpoint.
  Scalar getSetpoint() const;

  //! A method returning the generated thrust.
  Scalar getThrust() const;

  //! A method returning the induced torque.
  Scalar getTorque() const;

  //! A method returning the angular position of the propeller [rad]
  Scalar getAngle() const;

  //! A method returning the angular velocity of the propeller [rad/s]
  Scalar getOmega() const;

  //! A method returning the maximum speed of the thruster [RPM].
  Scalar getMaxRPM() const;

  //! A method informing if the propeller is right-handed.
  bool isPropellerRight() const;

  //! A method returning the type of the actuator.
  ActuatorType getType() const;

private:
  void WatchdogTimeout() override;

  // Params
  SolidEntity* prop_;
  bool RH_;
  bool inv_;
  std::pair<Scalar, Scalar> limits_;

  // States
  Scalar theta_;   // [rad]. Angle of the propeller
  Scalar omega_;   // [rad/s]. Angular velocity of the propeller
  Scalar thrust_;  // [N]. Generated thrust
  Scalar torque_;  // [Nm]. Induced torque @TODO

  Scalar setpoint_;  // [-]. Desired speed of the propeller

  // Dynamics
  std::shared_ptr<rm::RotorDynamics> rotorModel_;
  std::shared_ptr<tm::ThrustModel> thrustModel_;
};
}  // namespace sf
