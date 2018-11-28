//
//  FOG.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FOG__
#define __Stonefish_FOG__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class FOG : public LinkSensor
{
public:
    FOG(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    void SetNoise(Scalar headingStdDev);
};
    
}

#endif
