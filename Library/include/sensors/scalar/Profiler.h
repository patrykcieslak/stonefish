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
//  Profiler.h
//  Stonefish
//
//  Created by Patryk Cieslak on 31/07/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a single beam profiler.
    class Profiler : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param angleRangeDeg the field of view of the profiler [deg]
         \param angleSteps the resolution of the profiler
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Profiler(const std::string& uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt) override;
        
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
        ScalarSensorType getScalarSensorType() const override;
        
    private:
        Scalar angRange_;
        unsigned int angSteps_;
        unsigned int currentAngStep_;
        Scalar distance_;
        bool clockwise_;
    };
}
