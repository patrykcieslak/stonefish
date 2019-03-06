//
//  Pressure.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Pressure__
#define __Stonefish_Pressure__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a pressure sensor.
    class Pressure : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Pressure(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param max the maximum measured pressure [Pa]
         */
        void setRange(Scalar max);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param pressureStdDev standard deviation of the pressure measurement noise
         */
        void setNoise(Scalar pressureStdDev);
    };
}

#endif
