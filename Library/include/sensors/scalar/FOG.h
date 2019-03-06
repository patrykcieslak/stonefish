//
//  FOG.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FOG__
#define __Stonefish_FOG__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a fiber optic gyro (FOG) or compass.
    class FOG : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        FOG(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param headingStdDev standard deviation of the heading measurement noise
         */
        void setNoise(Scalar headingStdDev);
    };
}

#endif
