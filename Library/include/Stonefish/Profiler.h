//
//  Profiler.h
//  Stonefish
//
//  Created by Patryk Cieslak on 31/07/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Profiler__
#define __Stonefish_Profiler__

#include "SimpleSensor.h"
#include "SolidEntity.h"

class Profiler : public SimpleSensor
{
public:
    Profiler(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar angleRangeDeg, unsigned int angleSteps, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    std::vector<Renderable> Render();
    
    void SetRange(btScalar distanceMin, btScalar distanceMax);
    void SetNoise(btScalar distanceStdDev);
    btTransform getSensorFrame();
    
private:
    SolidEntity* attach;
    btScalar angRange;
    unsigned int angSteps;
    unsigned int currentAngStep;
    btScalar distance;
    bool clockwise;
};

#endif
