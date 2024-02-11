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
//  Multibeam.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Multibeam__
#define __Stonefish_Multibeam__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a multibeam (raycasting, single plane of beams).
    class Multibeam : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param angleRangeDeg a field of view of the multibeam [deg]
         \param angleSteps resolution of the multibeam
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Multibeam(std::string uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param rangeMin the minimum measured range [m]
         \param rangeMax the maximum measured range [m]
         */
        void setRange(Scalar rangeMin, Scalar rangeMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param rangeStdDev standard deviation of the range measurement noise
         */
        void setNoise(Scalar rangeStdDev);
        
        //! A method resetting the state of the sensor.
        std::vector<Renderable> Render();
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();

        //! A method returning the angleRangeDeg parameter
        Scalar getAngleRange();
        
    private:
        Scalar angRange;
        unsigned int angSteps;
        std::vector<Scalar> angles;
        std::vector<Scalar> distances;
    };
}

#endif
