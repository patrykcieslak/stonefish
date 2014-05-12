//
//  FakeIMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FakeIMU__
#define __Stonefish_FakeIMU__

#include "Sensor.h"
#include "SolidEntity.h"

class FakeIMU : public Sensor
{
public:
    FakeIMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, unsigned int historyLength = 1);
    
    void Reset();
    void Update(btScalar dt);
    unsigned short getNumOfDimensions();
    
private:
    //parameters
    SolidEntity* solid;
    btTransform relToSolid;
    
    //temporary
    btVector3 lastV;
};

#endif
