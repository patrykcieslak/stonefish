//
//  Magnetometer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Magnetometer__
#define __Stonefish_Magnetometer__

#include "Sensor.h"
#include "ADC.h"
#include "SolidEntity.h"

class Magnetometer : public Sensor
{
public:
    Magnetometer(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, unsigned short resolution, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    unsigned short getNumOfDimensions();
    
private:
    //parameters
    SolidEntity* solid;
    btTransform relToSolid;
    AxisType axis;
    unsigned short bits;
};

#endif
