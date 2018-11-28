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
    Profiler(std::string uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    void SetRange(Scalar distanceMin, Scalar distanceMax);
    void SetNoise(Scalar distanceStdDev);
    std::vector<Renderable> Render();
    
private:
    Scalar angRange;
    unsigned int angSteps;
    unsigned int currentAngStep;
    Scalar distance;
    bool clockwise;
};
    
}

#endif
