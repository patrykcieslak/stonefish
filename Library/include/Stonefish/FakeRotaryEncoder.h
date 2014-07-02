//
//  FakeRotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FakeRotaryEncoder__
#define __Stonefish_FakeRotaryEncoder__

#include "Sensor.h"
#include "RevoluteJoint.h"
#include "FeatherstoneEntity.h"

class FakeRotaryEncoder : public Sensor
{
public:
    FakeRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    FakeRotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    unsigned short getNumOfDimensions();
        
private:
    btScalar getRawAngle();
    btScalar getRawAngularVelocity();
    
    //params
    RevoluteJoint* revolute;
    FeatherstoneEntity* multibody;
    unsigned int multibodyJoint;
    
    //temporary
    btScalar angle;
    btScalar angularVelocity;
    btScalar lastAngle;
};



#endif
