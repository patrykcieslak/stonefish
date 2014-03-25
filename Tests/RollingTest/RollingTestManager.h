//
//  TestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__RollingTestManager__
#define __Stonefish__RollingTestManager__

#include "SimulationManager.h"

class RollingTestManager : public SimulationManager
{
public:
    RollingTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif