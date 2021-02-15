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
//  Actuator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Actuator__
#define __Stonefish_Actuator__

#include "StonefishCommon.h"
#include "entities/Entity.h"

namespace sf
{
    struct Renderable;
    
    //! An enum designating a type of the actuator.
    enum class ActuatorType {MOTOR, SERVO, PROPELLER, THRUSTER, VBS, LIGHT, RUDDER};
    
    //! An abstract class representing any actuator.
    class Actuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name of the actuator
         */
        Actuator(std::string uniqueName);
        
        //! A destructor.
        virtual ~Actuator();
        
        //! A method used to update the internal state of the actuator.
        /*!
         \param dt a time step of the simulation [s]
         */
        virtual void Update(Scalar dt) = 0;
        
        //! A method implementing the rendering of the actuator.
        virtual std::vector<Renderable> Render();
        
        //! A method used to set display mode used for the actuator.
        /*!
         \param m flag defining the display mode
         */
        void setDisplayMode(DisplayMode m);

        //! A method returning the type of the actuator.
        virtual ActuatorType getType() = 0;

        //! A method returning the name of the actuator.
        std::string getName();
    
    protected:
        DisplayMode dm;

    private:
        std::string name;
    };
}

#endif 
