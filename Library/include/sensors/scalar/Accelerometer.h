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
    Accelerometer(std::string uniqueName, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar linearAccMax, btScalar angularAccMax);
    void SetNoise(btScalar linearAccStdDev, btScalar angularAccStdDev);
};
    
}

#endif
