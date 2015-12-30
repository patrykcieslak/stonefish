//
//  AcroverSimManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcroverSimManager__
#define __Stonefish__AcroverSimManager__

#include "SimulationManager.h"
#include "PathGenerator.h"

class AcroverSimManager : public SimulationManager
{
public:
    AcroverSimManager(btScalar stepsPerSecond);
    
    void BuildScenario();
    PathGenerator* getPath();
};

#endif