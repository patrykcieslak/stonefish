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
//  Push.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/07/2023.
//  Copyright (c) 2023-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Push__
#define __Stonefish_Push__

#include "actuators/LinkActuator.h"

namespace sf
{
    //! A class representing a thruster.
    class Push : public LinkActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the push
         \param inverted a flag indicating if the direction of the generated force should be reversed
         \param onlyWorksSubmerged a flag indicating if the actuator is simulating an underwater thruster
        */
        Push(std::string uniqueName, bool inverted = false, bool onlyWorksSubmerged = false);
        
        //! A method used to update the internal state of the push actuator.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method implementing the rendering of the push actuator.
        std::vector<Renderable> Render();
        
        //! A method used to set the force limits.
        void setForceLimits(double lower, double upper);

        //! A method setting the new value of the desired force.
        /*!
         \param f the desired force to be generated
         */
        void setForce(Scalar f);
        
        //! A method returning the current setpoint.
        Scalar getForce() const;
        
        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
    private:
        void WatchdogTimeout() override;

        Scalar setpoint;
        bool underwater;
        bool inv;
        std::pair<double, double> limits;
    };
}

#endif
