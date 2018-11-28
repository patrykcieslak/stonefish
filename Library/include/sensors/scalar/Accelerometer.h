//
//  Accelerometer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Accelerometer__
#define __Stonefish_Accelerometer__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class Accelerometer : public LinkSensor
{
public:
    Accelerometer(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    void SetRange(Scalar linearAccMax, Scalar angularAccMax);
    void SetNoise(Scalar linearAccStdDev, Scalar angularAccStdDev);
};
    
}

#endif
