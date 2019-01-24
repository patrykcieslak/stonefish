//
//  IMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMU__
#define __Stonefish_IMU__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing an inertial measurement unit (IMU).
    class IMU : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        IMU(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param angularVelocityMax the maximum measured angular velocity [rad s^-1]
         */
        void SetRange(Scalar angularVelocityMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param angleStdDev standard deviation of the angle measurement noise
         \param angularVelocityStdDev standard deviation of the angular velocity measurement noise
         */
        void SetNoise(Scalar angleStdDev, Scalar angularVelocityStdDev);
    };
}

#endif
