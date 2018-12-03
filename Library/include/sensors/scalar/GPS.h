//
//  GPS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GPS__
#define __Stonefish_GPS__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    class NED;
    
    //!
    class GPS : public LinkSensor
    {
    public:
        GPS(std::string uniqueName, Scalar latitudeDeg, Scalar longitudeDeg, Scalar frequency = Scalar(-1), int historyLength = -1);
        ~GPS();
        
        void InternalUpdate(Scalar dt);
        void SetNoise(Scalar nedDev);
        
    private:
        NED* ned;
        
        //Custom noise generation specific to GPS
        Scalar nedStdDev;
        std::normal_distribution<Scalar> noise;
    };
}

#endif
