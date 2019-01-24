//
//  GPS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GPS__
#define __Stonefish_GPS__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    class NED;
    
    //! A class representing a global positioning system (GPS) sensor.
    class GPS : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param latitudeDeg latitude of the world origin [deg]
         \param longitudeDeg longitude of the world origin [deg]
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        GPS(std::string uniqueName, Scalar latitudeDeg, Scalar longitudeDeg, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A destructor.
        ~GPS();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param nedDev standard deviation of the NED position measurement noise
         */
        void SetNoise(Scalar nedDev);
        
    private:
        NED* ned;
        
        //Custom noise generation specific to GPS
        Scalar nedStdDev;
        std::normal_distribution<Scalar> noise;
    };
}

#endif
