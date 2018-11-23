//
//  Pressure.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Pressure__
#define __Stonefish_Pressure__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class Pressure : public LinkSensor
{
public:
    Pressure(std::string uniqueName, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar max);
    void SetNoise(btScalar pressureStdDev);
};
    
}

#endif
