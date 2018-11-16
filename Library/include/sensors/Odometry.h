//
//  Odometry.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Odometry__
#define __Stonefish_Odometry__

#include "sensors/SimpleSensor.h"

class Odometry : public SimpleSensor
{
public:
    Odometry(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetNoise(btScalar positionStdDev, btScalar velocityStdDev, btScalar orientationStdDev, btScalar angularVelocityStdDev);
    btTransform getSensorFrame();
    
protected:
    SolidEntity* attach;
};

#endif
