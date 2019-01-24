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
