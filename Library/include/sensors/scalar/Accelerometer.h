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
//  Accelerometer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/11/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Accelerometer__
#define __Stonefish_Accelerometer__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing an accelerometer.
    class Accelerometer : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Accelerometer(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param linearAccMax the maximum measured linear acceleration [m s^-2]
         \param angularAccMax the maximum measured angular acceleration [rad s^-2]
         */
        void setRange(Scalar linearAccMax, Scalar angularAccMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param linearAccStdDev standard deviation of the linear acceleration measurement noise
         \param angularAccStdDev standard deviation of the angular acceleration measurement noise
         */
        void setNoise(Scalar linearAccStdDev, Scalar angularAccStdDev);
    };
}

#endif
