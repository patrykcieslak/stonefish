//
//  Multibeam.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Multibeam__
#define __Stonefish_Multibeam__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class Multibeam : public LinkSensor
{
public:
    Multibeam(std::string uniqueName, btScalar angleRangeDeg, unsigned int angleSteps, btScalar frequency = btScalar(-1), int historyLength = -1);
   
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar distanceMin, btScalar distanceMax);
    void SetNoise(btScalar distanceStdDev);
    std::vector<Renderable> Render();
    
private:
    btScalar angRange;
    unsigned int angSteps;
    std::vector<btScalar> angles;
    std::vector<btScalar> distances; 
};
    
}

#endif
