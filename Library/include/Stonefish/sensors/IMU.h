//
//  IMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMU__
#define __Stonefish_IMU__

#include "SimpleSensor.h"

class IMU : public SimpleSensor
{
public:
    IMU(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar angularVelocityMax);
    void SetNoise(btScalar angleStdDev, btScalar angularVelocityStdDev);
    btTransform getSensorFrame();
    
protected:
    SolidEntity* attach;
};

#endif
