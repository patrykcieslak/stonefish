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
    IMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, unsigned int historyLength = 1);
    
    void Reset();
    void Update(btScalar dt);
    unsigned short getNumOfDimensions();
    
protected:
    //parameters
    SolidEntity* solid;
    btTransform relToSolid;
};

#endif
