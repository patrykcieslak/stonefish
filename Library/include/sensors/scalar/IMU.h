//
//  IMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMU__
#define __Stonefish_IMU__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class IMU : public LinkSensor
{
public:
    IMU(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    void SetRange(Scalar angularVelocityMax);
    void SetNoise(Scalar angleStdDev, Scalar angularVelocityStdDev);
};
    
}

#endif
