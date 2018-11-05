//
//  Multibeam.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Multibeam__
#define __Stonefish_Multibeam__

#include "SimpleSensor.h"

class Multibeam : public SimpleSensor
{
public:
    Multibeam(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar angleRangeDeg, unsigned int angleSteps, btScalar frequency = btScalar(-1.), int historyLength = -1);
   
    void InternalUpdate(btScalar dt);
    std::vector<Renderable> Render();
    
    void SetRange(btScalar distanceMin, btScalar distanceMax);
    void SetNoise(btScalar distanceStdDev);
    btTransform getSensorFrame();
    
private:
    SolidEntity* attach;
    btScalar angRange;
    unsigned int angSteps;
    std::vector<btScalar> angles;
    std::vector<btScalar> distances; 
};

#endif
