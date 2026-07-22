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
//  Rudder.h
//  Stonefish
//
//  Created by Nils Bore on 29/01/2021.
//  Copyright (c) 2021-2026 Nils Bore, Patryk Cieslak. All rights reserved.
//

#pragma once

#include "actuators/LinkActuator.h"

namespace sf
{
    //! A class representing a rudder working in water.
    class Rudder : public LinkActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the rudder
         \param rudder a pointer to a rigid body representing the rudder
         \param area the area of the rudder [m^2]
         \param liftCoeff the lift coefficient
         \param dragCoeff the drag coefficient
         \param stallAngle the angle of attack above which lift ceases to be generated [rad]
         \param maxAngle the maximum angle of the rudder [rad]
         \param inverted a flag to indicate if the setpoint is inverted (positive value results in left-handed rotation)
        */
        Rudder(const std::string& uniqueName, std::unique_ptr<SolidEntity> rudder, Scalar area, Scalar liftCoeff, Scalar dragCoeff, Scalar stallAngle, 
            Scalar maxAngle, bool inverted = false, Scalar maxAngularRate = Scalar(0));
        
        //! A method used to update the internal state of the rudder.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method implementing the rendering of the rudder.
        std::vector<Renderable> Render();
        
        //! A method setting the new value of the rudder angle setpoint.
        /*!
         \param s the desired angle of the rudder as an angle [rad]
         */
        void setSetpoint(Scalar s);
        
        //! A method returning the current setpoint.
        Scalar getSetpoint() const;

        //! A method returning the angular position of the rudder [rad]
        Scalar getAngle() const;
        
        //! A method returning type of link actuator.
        LinkActuatorType getLinkActuatorType() const override;

        //! A method returning the construction info for the actuator.
        static ConstructInfo getConstructInfo();

        //! A method constructing the actuator based on info structure.
        /*!
         \param info a construction info structure
        */
        static std::unique_ptr<Rudder> Construct(const std::string& uniqueName, ConstructInfo& info);
        
    private:
        //Params
        Scalar dragCoeff_;
        Scalar liftCoeff_;
        Scalar area_;
        Scalar stallAngle_;
        Scalar maxAngle_;
        Scalar maxAngularRate_;
        std::unique_ptr<SolidEntity> rudder_;
        bool inv_;
        
        //States
        Scalar theta_;
        Scalar setpoint_;
        Vector3 liftV_;
        Vector3 dragV_;
    };
}

