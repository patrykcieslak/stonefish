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
//  Propeller.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "actuators/LinkActuator.h"

namespace sf
{
    //! A class representing a propeller working in air, driven by a motor.
    class Propeller : public LinkActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the thruster
         \param propeller a pointer to a rigid body representing the propeller
         \param diameter the diameter of the propeller [m]
         \param thrustCoeff the thrust coefficient
         \param torqueCoeff the torque coefficient
         \param maxRPM the maximum rotational speed of the motor [rpm]
         \param rightHand a flag to indicate if the propeller is right hand (clockwise rotation)
         \param inverted a flag to indicate if the setpoint is inverted (positive value results in backward force)
        */
        Propeller(const std::string& uniqueName, std::unique_ptr<SolidEntity> propeller, Scalar diameter, Scalar thrustCoeff, Scalar torqueCoeff, Scalar maxRPM, bool rightHand, bool inverted = false);
        
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
        
        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
    private:
        void WatchdogTimeout() override;

        //Params
        Scalar D_;
        Scalar kT0_;
        Scalar kQ0_;
        Scalar kp_;
        Scalar ki_;
        Scalar iLim_;
        Scalar omegaLim_;
        std::unique_ptr<SolidEntity> propeller_;
        bool RH_;
        bool inv_;
        
        //States
        Scalar theta_;
        Scalar omega_;
        Scalar thrust_;
        Scalar torque_;
        Scalar setpoint_;
        Scalar iError_;
    };
}
