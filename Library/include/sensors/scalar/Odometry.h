//
//  Odometry.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Odometry__
#define __Stonefish_Odometry__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a sensor that captures the pose and velocities of a body.
    class Odometry : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        Odometry(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param positionStdDev standard deviation of the position measurement noise
         \param velocityStdDev standard deviation of the linear velocity measurement noise
         \param orientationStdDev standard deviation of the orientation measurement noise
         \param angularVelocityStdDev standard deviation of the angular velocity measurement noise
         */
        void setNoise(Scalar positionStdDev, Scalar velocityStdDev, Scalar orientationStdDev, Scalar angularVelocityStdDev);
    };
}
    
#endif
