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
//  SuctionCup.h
//  Stonefish
//
//  Created by Patryk Cieslak on 13/02/2023.
//  Copyright (c) 2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SuctionCup__
#define __Stonefish_SuctionCup__

#include "actuators/LinkActuator.h"

namespace sf
{
    class SimulationManager;
    class SpringJoint;

    //! A class representing a thruster.
    class SuctionCup : public LinkActuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the suction cup actuator
        */
        SuctionCup(std::string uniqueName);
        
        //! A destructor.
        ~SuctionCup();

        //! A method used to attach the actuator to a specified rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body origin to the actuator origin
         */
        void AttachToSolid(SolidEntity* body, const Transform& origin);
        
        //! A method used to update the internal state of the thruster.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);

        //!
        void Engage(SimulationManager* sm);
        
        //!  
        void setPump(bool enabled);

        //!
        bool getPump() const;

        //! A method returning the type of the actuator.
        ActuatorType getType() const;
        
    private:
        bool pump;
        SpringJoint* spring;
    };
}

#endif
