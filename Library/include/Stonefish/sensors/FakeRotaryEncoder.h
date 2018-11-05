//
//  FakeRotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FakeRotaryEncoder__
#define __Stonefish_FakeRotaryEncoder__

#include "RotaryEncoder.h"

class FakeRotaryEncoder : public RotaryEncoder
{
public:
    FakeRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency = btScalar(-1.), int historyLength = -1);
    FakeRotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency = btScalar(-1.), int historyLength = -1);
    FakeRotaryEncoder(std::string uniqueName, Motor* m, btScalar frequency = btScalar(-1.), int historyLength = -1);
    FakeRotaryEncoder(std::string uniqueName, Thruster* m, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void Reset();
        
private:
    btScalar angle;
    btScalar angularVelocity;
    btScalar lastAngle;
};



#endif
