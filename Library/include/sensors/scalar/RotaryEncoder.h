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
//  RotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RotaryEncoder__
#define __Stonefish_RotaryEncoder__

#include "sensors/scalar/JointSensor.h"

namespace sf
{
    class Motor;
    class Thruster;
    
    //! A class representing a rotary encoder with "infinite" resolution.
    class RotaryEncoder : public JointSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        RotaryEncoder(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method used to attach the encoder to a motor.
        /*!
         \param m a pointer to the motor
         */
        void AttachToMotor(Motor* m);
        
        //! A method used to attach the encoder to the thruster.
        /*!
         \param th a pointer to the thruster
         */
        void AttachToThruster(Thruster* th);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt);
        
        //! A method that resets the sensor.
        virtual void Reset();
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();
        
    protected:
        Scalar GetRawAngle();
        Scalar GetRawAngularVelocity();
        
        Motor* motor;
        Thruster* thrust;
        Scalar angle;
        Scalar lastAngle;
    };
}

#endif
