//
//  IMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMU__
#define __Stonefish_IMU__

#include "SimpleSensor.h"
#include "SolidEntity.h"

class IMU : public SimpleSensor
{
public:
    IMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    virtual void InternalUpdate(btScalar dt) = 0;
    virtual void Reset() = 0;
    
protected:
    SolidEntity* solid;
    btTransform relToSolid;
};

#endif
