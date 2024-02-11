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
//  LinkSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_LinkSensor__
#define __Stonefish_LinkSensor__

#include "sensors/ScalarSensor.h"

namespace sf
{
    class MovingEntity;
    class FeatherstoneEntity;
    
    //! An abstract class representing a sensor attached to a rigid body.
    class LinkSensor : public ScalarSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (0 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        LinkSensor(std::string uniqueName, Scalar frequency, int historyLength);
        
        //! A destructor.
        virtual ~LinkSensor();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method implementing the rendering of the sensor.
        virtual std::vector<Renderable> Render();
      
        //! A method used to attach the sensor to a rigid body.
        /*!
         \param solid a pointer to a rigid body
         \param origin a transformation from the body frame to the sensor frame
         */
        void AttachToSolid(MovingEntity* solid, const Transform& origin);
        
        //! A method used to set the sensor frame in the body frame.
        /*!
         \param origin a tranformation from the body frame to the sensor frame
         */
        void setRelativeSensorFrame(const Transform& origin);

        //! A method returning the type of the sensor.
        SensorType getType();

        //! A method returning the current sensor frame in world.
        virtual Transform getSensorFrame() const;
        
        //! A method returning the name of the link that the sensor is attached to.
        std::string getLinkName();
        
    protected:
        MovingEntity* attach;
        Transform o2s;
    };
}

#endif
