//
//  Odometry.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Odometry__
#define __Stonefish_Odometry__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //!
    class Odometry : public LinkSensor
    {
    public:
        Odometry(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        void InternalUpdate(Scalar dt);
        void SetNoise(Scalar positionStdDev, Scalar velocityStdDev, Scalar orientationStdDev, Scalar angularVelocityStdDev);
    };
}
    
#endif
