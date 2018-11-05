//
//  FOG.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FOG__
#define __Stonefish_FOG__

#include "SimpleSensor.h"

class FOG : public SimpleSensor
{
public:
    FOG(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetNoise(btScalar headingStdDev);
    btTransform getSensorFrame();
    
private:
    SolidEntity* attach;
};

#endif