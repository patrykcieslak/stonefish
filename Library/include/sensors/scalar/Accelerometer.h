//
//  Accelerometer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/11/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Accelerometer__
#define __Stonefish_Accelerometer__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing an accelerometer.
    class Accelerometer : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Accelerometer(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param linearAccMax the maximum measured linear acceleration [m s^-2]
         \param angularAccMax the maximum measured angular acceleration [rad s^-2]
         */
        void setRange(Scalar linearAccMax, Scalar angularAccMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param linearAccStdDev standard deviation of the linear acceleration measurement noise
         \param angularAccStdDev standard deviation of the angular acceleration measurement noise
         */
        void setNoise(Scalar linearAccStdDev, Scalar angularAccStdDev);
    };
}

#endif
