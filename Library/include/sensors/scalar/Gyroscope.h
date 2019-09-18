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
//  Gyroscope.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Gyroscope__
#define __Stonefish_Gyroscope__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    class ADC;
    
    //! A class representing a realistic gyroscope.
    class Gyroscope : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param rangeMin minimum measured angular velocity [rad/s]
         \param rangeMax maximum measured angular velocity [rad/s]
         \param sensitivity sensor sensitivity [rad/V]
         \param zeroVoltage the offset of the zero measurement [V]
         \param driftSpeed a factor defining how fast the gyroscope drifts
         \param adc a pointer to an analog-digital converter
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Gyroscope(std::string uniqueName, Scalar rangeMin, Scalar rangeMax, Scalar sensitivity, Scalar zeroVoltage,
                  Scalar driftSpeed, ADC* adc, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method resetting the state of the sensor.
        void Reset();
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();
        
    private:
        Scalar range[2];
        Scalar sens;
        Scalar zeroV;
        Scalar drift;
        ADC* adc;
        
        //temporary
        Scalar accumulatedDrift;
    };
}

#endif
