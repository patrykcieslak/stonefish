//
//  JointsTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__JointsTestManager__
#define __Stonefish__JointsTestManager__

#include "SimulationManager.h"

class JointsTestManager : public SimulationManager
{
public:
    JointsTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif