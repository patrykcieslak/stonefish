//
//  DVL.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DVL__
#define __Stonefish_DVL__

#include "SimpleSensor.h"
#include "SolidEntity.h"

class DVL : public SimpleSensor
{
public:
    DVL(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    std::vector<Renderable> Render();
    
    void SetRange(const btVector3& velocityMax, btScalar altitudeMin, btScalar altitudeMax);
    void SetNoise(btScalar velocityStdDev, btScalar altitudeStdDev);
    btTransform getSensorFrame();
    
private:
    SolidEntity* attach;
};

#endif
