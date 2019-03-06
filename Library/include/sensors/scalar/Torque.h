//
//  Torque.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Torque__
#define __Stonefish_Torque__

#include "sensors/scalar/JointSensor.h"

namespace sf
{
    //! A class representing a torque sensor.
    class Torque : public JointSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Torque(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param max the maximum measured torque [Nm]
         */
        void setRange(Scalar max);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param stdDev standard deviation of torque measurement noise
         */
        void setNoise(Scalar stdDev);
    };
}

#endif
