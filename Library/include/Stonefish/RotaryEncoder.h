//
//  RotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RotaryEncoder__
#define __Stonefish_RotaryEncoder__

#include "SimpleSensor.h"
#include "RevoluteJoint.h"
#include "FeatherstoneEntity.h"

class RotaryEncoder : public SimpleSensor
{
public:
    RotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    RotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    virtual void InternalUpdate(btScalar dt) = 0;
    virtual void Reset() = 0;
    
protected:
    btScalar GetRawAngle();
    
    RevoluteJoint* revolute;
    FeatherstoneEntity* multibody;
    unsigned int multibodyJoint;
};

#endif
