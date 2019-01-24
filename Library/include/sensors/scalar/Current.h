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
        
    private:
        DCMotor* motor;
    };
}

#endif
