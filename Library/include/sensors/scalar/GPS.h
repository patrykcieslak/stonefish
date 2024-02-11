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
//  GPS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GPS__
#define __Stonefish_GPS__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a global positioning system (GPS) sensor.
    class GPS : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        GPS(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param nedDev standard deviation of the NED position measurement noise [m]
         */
        void setNoise(Scalar nedDev);
        
        //! A method that returns the standard deviation of position in meters.
        Scalar getNoise();
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();
        
    private:
        //Custom noise generation specific to GPS
        Scalar nedStdDev;
        std::normal_distribution<Scalar> noise;
    };
}

#endif
