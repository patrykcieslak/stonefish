//
//  DVL.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DVL__
#define __Stonefish_DVL__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a Dopple velocity log (DVL) sensor with four beams.
    class DVL : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param beamSpreadAngleDeg the angle between the beams [deg]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        DVL(std::string uniqueName, Scalar beamSpreadAngleDeg, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param velocityMax the maximum measured linear velocity [m s^-1]
         \param altitudeMin the minimum measured altitude [m]
         \param altitudeMax the maximum measured altitude [m]
         */
        void setRange(const Vector3& velocityMax, Scalar altitudeMin, Scalar altitudeMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param velocityStdDev standard deviation of the linear velocity measurement noise
         \param altitudeStdDev standard deviation of the altitude measurement noise
         */
        void setNoise(Scalar velocityStdDev, Scalar altitudeStdDev);
        
        //! A method resetting the state of the sensor.
        std::vector<Renderable> Render();
        
    private:
        Scalar beamAngle;
        Scalar range[4];
    };
}

#endif
