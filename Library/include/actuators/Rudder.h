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
//  Copyright (c) 2021-2023 Nils Bore, Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Rudder__
#define __Stonefish_Rudder__

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
        Rudder(std::string uniqueName, SolidEntity* rudder, Scalar area, Scalar liftCoeff, Scalar dragCoeff, Scalar stallAngle, Scalar maxAngle, bool inverted = false);
        
        //! A destructor.
        ~Rudder();
        
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
        
        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
    private:
        //Params
        Scalar dragCoeff;
        Scalar liftCoeff;
        Scalar area;
        Scalar stallAngle;
        Scalar maxAngle;
        SolidEntity* rudder;
        bool inv;
        
        //States
        Scalar theta;
        Scalar setpoint;
        Vector3 liftV;
        Vector3 dragV;
    };
}

#endif
