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
//  Thruster.h
//  Stonefish
//
//  Created by Roger Pi on 03/06/2024
//  Modified by Patryk Cieslak on 30/06/2024
//  Copyright (c) 2024-2025 Roger Pi and Patryk Cieslak. All rights reserved.
//

#pragma once

#include "actuators/LinkActuator.h"
#include "ActuatorDynamics.h"

namespace sf
{
//! A class representing a thruster.
class Thruster : public LinkActuator
{
public:
  //! A constructor.
  /*!
   \param uniqueName a name for the thruster
   \param propeller a pointer to a rigid body representing the propeller
   \param rotorDynamics a pointer to the rotor dynamics model
   \param thrustConversion a pointer to the thrust conversion model
   \param diameter diameter of the propeller [m]
   \param rightHand a flag to indicate if the propeller is right hand (clockwise rotation)
   \param maxSetpoint limit of the thruster setpoint
   \param invertedSetpoint a flag to indicate if the setpoint is inverted (positive value results in backward force)
   \param normalizedSetpoint a flag to indicate if the setpoint given by the user is normalized [-1,1] 
  */
  Thruster(std::string uniqueName, std::shared_ptr<SolidEntity> propeller,     
                       std::shared_ptr<RotorDynamics> rotorDynamics,   
                       std::shared_ptr<ThrustModel> thrustConversion,  
                       Scalar diameter, bool rightHand, Scalar maxSetpoint, 
                       bool invertedSetpoint = false,
                       bool normalizedSetpoint = true);

  //! A method used to update the internal state of the thruster.
  /*!
   \param dt a time step of the simulation [s]
   */
  void Update(Scalar dt);

  //! A method implementing the rendering of the thruster.
  std::vector<Renderable> Render();

  //! A method setting the new value of the thruster speed setpoint.
  /*!
   \param s the desired setpoint
   */
  void setSetpoint(Scalar s);

  //! A method used to set the thruster setpoint limit.
  /*!
    \param limit limit of the thruster setpoint
  */
  void setSetpointLimit(Scalar limit);

  //! A method returning the limit of the thruster setpoint.
  Scalar getSetpointLimit();

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

  //! A method informing if the propeller is right-handed.
  bool isPropellerRight() const;

  //! A method returning the diameter of the propeller.
  Scalar getPropellerDiameter() const;

  //! A method returning the type of the actuator.
  ActuatorType getType() const;

private:
  void WatchdogTimeout() override;

  // Params
  std::shared_ptr<SolidEntity> propeller_;
  bool RH;
  Scalar D;

  // States
  Scalar theta;   // Angle of the propeller [rad]
  Scalar omega;   // Angular velocity of the propeller [rad/s]
  Scalar thrust;  // Generated thrust [N]
  Scalar torque;  // Induced torque [Nm]

  Scalar setpoint;  // Desired setpoint
  Scalar setpointLimit; // Limit of the desired setpoint
  bool inv; // Is setpoint value inverted?
  bool normalized; // Is setpoint normalized?
  
  // Dynamics
  std::shared_ptr<RotorDynamics> rotorModel;
  std::shared_ptr<ThrustModel> thrustModel;
};
}  // namespace sf
