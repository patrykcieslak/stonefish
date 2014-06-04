//
//  SlidingTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__SlidingTestManager__
#define __Stonefish__SlidingTestManager__

#include "SimulationManager.h"

class SlidingTestManager : public SimulationManager
{
public:
    SlidingTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif