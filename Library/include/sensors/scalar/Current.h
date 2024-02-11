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
//  Current.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Current__
#define __Stonefish_Current__

#include "sensors/ScalarSensor.h"

namespace sf
{
    class DCMotor;
    
    //! A class representing an electrical current sensor.
    class Current : public ScalarSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Current(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to attach the sensor to a DC motor.
        /*!
         \param m pointer to a DC motor
         */
        void AttachToMotor(DCMotor* m);
        
        //! A method returning the type of the sensor.
        SensorType getType();
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();
        
    private:
        DCMotor* motor;
    };
}

#endif
