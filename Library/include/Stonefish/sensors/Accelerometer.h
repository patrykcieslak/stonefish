//
//  Accelerometer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Accelerometer__
#define __Stonefish_Accelerometer__

#include "SimpleSensor.h"

class Accelerometer : public SimpleSensor
{
public:
    Accelerometer(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar linearAccMax, btScalar angularAccMax);
    void SetNoise(btScalar linearAccStdDev, btScalar angularAccStdDev);
    btTransform getSensorFrame();
    
protected:
    SolidEntity* attach;
};

#endif
