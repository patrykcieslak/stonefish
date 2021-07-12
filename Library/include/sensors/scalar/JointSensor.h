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
//  JointSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_JointSensor__
#define __Stonefish_JointSensor__

#include "sensors/ScalarSensor.h"

namespace sf
{
    class FeatherstoneEntity;
    class Joint;
    
    //! An abstract class representing a sensor attached to a joint.
    class JointSensor : public ScalarSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (0 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        JointSensor(std::string uniqueName, Scalar frequency, int historyLength);
        
        //! A method performing internal update of the sensor state.
        /*!
         \param dt the sample time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method used to attach the sensor to a multibody joint.
        /*!
         \param multibody a pointer to a multibody
         \param jointId an index of the multibody joint
         */
        virtual void AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId);
        
        //! A method used to attach the sensor to a discrete joint.
        /*!
         \param joint a pointer to a discrete joint
         */
        virtual void AttachToJoint(Joint* joint);
        
        //! A method returning the type of the sensor.
        SensorType getType();
        
        //! A method returning the current sensor frame in world.
        virtual Transform getSensorFrame() const;
        
        //! A method returning the name of the joint that the sensor is attached to.
        std::string getJointName();
        
    protected:
        FeatherstoneEntity* fe;
        unsigned int jId;
        Joint* j;
    };
}

#endif
