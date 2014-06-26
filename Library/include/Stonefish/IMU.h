//
//  IMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMU__
#define __Stonefish_IMU__

#include "Sensor.h"
#include "SolidEntity.h"

class IMU : public Sensor
{
public:
    IMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    unsigned short getNumOfDimensions();
    
protected:
    //parameters
    SolidEntity* solid;
    btTransform relToSolid;
};

#endif
