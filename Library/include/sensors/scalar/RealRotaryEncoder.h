//
//  RealRotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RealRotaryEncoder__
#define __Stonefish_RealRotaryEncoder__

#include "sensors/scalar/RotaryEncoder.h"

namespace sf
{

class RealRotaryEncoder : public RotaryEncoder
{
public:
    RealRotaryEncoder(std::string uniqueName, unsigned int cpr_resolution, bool absolute = false, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    void Reset();
    
private:
    unsigned int cpr_res;
    unsigned int abs;
};
    
}

#endif
