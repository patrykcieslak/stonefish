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
//  Odometry.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Odometry__
#define __Stonefish_Odometry__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a sensor that captures the pose and velocities of a body.
    class Odometry : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Odometry(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param positionStdDev standard deviation of the position measurement noise
         \param velocityStdDev standard deviation of the linear velocity measurement noise
         \param angleStdDev standard deviation of the angle measurement noise
         \param angularVelocityStdDev standard deviation of the angular velocity measurement noise
         */
        void setNoise(Scalar positionStdDev, Scalar velocityStdDev, Scalar angleStdDev, Scalar angularVelocityStdDev);
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();

    private:
        //Custom noise generation
        Scalar ornStdDev;
        std::normal_distribution<Scalar> ornNoise;
    };
}
    
#endif
