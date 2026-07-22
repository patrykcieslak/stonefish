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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "StonefishCommon.h"
#include "entities/Entity.h"
#include "core/ConstructInfo.h"

namespace sf
{
    struct Renderable;
    
    //! An enum defining types of sensors.
    enum class ActuatorType {JOINT, LINK};
    
    //! An abstract class representing any actuator.
    class Actuator
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name of the actuator
         */
        Actuator(const std::string& uniqueName);
        
        //! A destructor.
        virtual ~Actuator();
        
        //! A method used to update the internal state of the actuator.
        /*!
         \param dt a time step of the simulation [s]
         */
        virtual void Update(Scalar dt);
        
        //! A method implementing the rendering of the actuator.
        virtual std::vector<Renderable> Render();
        
        //! A method used to set display mode used for the actuator.
        /*!
         \param m flag defining the display mode
         */
        void setDisplayMode(DisplayMode m);

        //! A method used to set the watchdog timeout.
        /*!
         \param timeout timeout of the watchdog [s]
        */
        void setWatchdog(Scalar timeout);

        //! A method returning the name of the actuator.
        const std::string& getName() const;

        //! A method returning the type of the actuator.
        virtual ActuatorType getType() const = 0;
    
    protected:
        virtual void WatchdogTimeout();
        void ResetWatchdog();

        DisplayMode dm_;

    private:
        std::string name_;
        Scalar watchdog_;
        Scalar watchdogTimeout_;
    };
}
