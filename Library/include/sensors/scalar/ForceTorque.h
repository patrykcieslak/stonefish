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
//  ForceTorque.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ForceTorque__
#define __Stonefish_ForceTorque__

#include "sensors/scalar/JointSensor.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a 6-axis force/torque sensor.
    class ForceTorque : public JointSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param attachment a pointer to the solid used as reference
         \param origin frame of the sensor in the reference body frame
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        ForceTorque(std::string uniqueName, SolidEntity* attachment, const Transform& origin, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param origin frame of the sensor in the reference body frame
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        ForceTorque(std::string uniqueName, const Transform& origin, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param forceMax a vector representing the maximum measured forces [N]
         \param torqueMax a vector representing the maximum mesured torque [Nm]
         */
        void setRange(const Vector3& forceMax, const Vector3& torqueMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param forceStdDev standard deviation of force measurement noise
         \param toqueStdDev standard deviation of torque measurement noise
         */
        void setNoise(Scalar forceStdDev, Scalar torqueStdDev);
        
        //! A method that implements rendering of the sensor.
        std::vector<Renderable> Render();
        
        //! A method returning the current sensor frame in world.
        Transform getSensorFrame() const;
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();
        
    private:
        SolidEntity* attach;
        Transform o2s;
        Transform lastFrame;
    };
}

#endif
