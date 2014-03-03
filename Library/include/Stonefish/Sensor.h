//
//  Sensor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sensor__
#define __Stonefish_Sensor__

#include "common.h"

//pure virtual class
class Sensor
{
public:
    Sensor();
    virtual ~Sensor();
    
    virtual btScalar* getHistory();
    virtual short getNumOfDimensions() = 0;
    
private:
    short resolutionBits;
    btScalar quantumSize; //range divided by resolution
    btScalar rangeMin;
    btScalar rangeMax;
    btScalar sensitivity;
    btScalar* history;
};

#endif
