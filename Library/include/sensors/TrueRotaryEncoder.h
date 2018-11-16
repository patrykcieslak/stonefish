//
//  TrueRotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_TrueRotaryEncoder__
#define __Stonefish_TrueRotaryEncoder__

#include "sensors/RotaryEncoder.h"

class TrueRotaryEncoder : public RotaryEncoder
{
public:
    TrueRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, unsigned int cpr_resolution, bool absolute = false, btScalar frequency = btScalar(-1.), int historyLength = -1);
    TrueRotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, unsigned int cpr_resolution, bool absolute = false, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    
private:
    unsigned int cpr_res;
    unsigned int abs;
    
    btScalar angle;
    btScalar lastAngle;
};

#endif
