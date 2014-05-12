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

class FakeRotaryEncoder : public Sensor
{
public:
    FakeRotaryEncoder(std::string uniqueName, RevoluteJoint* joint, unsigned int historyLength = 1);
    void Reset();
    void Update(btScalar dt);
    unsigned short getNumOfDimensions();
        
private:
    //params
    RevoluteJoint* revolute;
    
    //temporary
    btScalar angle;
    btScalar angularVelocity;
    btScalar lastAngle;
};



#endif
