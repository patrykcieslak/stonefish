//
//  Profiler.h
//  Stonefish
//
//  Created by Patryk Cieslak on 31/07/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Profiler__
#define __Stonefish_Profiler__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class Profiler : public LinkSensor
{
public:
    Profiler(std::string uniqueName, btScalar angleRangeDeg, unsigned int angleSteps, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar distanceMin, btScalar distanceMax);
    void SetNoise(btScalar distanceStdDev);
    std::vector<Renderable> Render();
    
private:
    btScalar angRange;
    unsigned int angSteps;
    unsigned int currentAngStep;
    btScalar distance;
    bool clockwise;
};
    
}

#endif
