//
//  ConsoleTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/09/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__ConsoleTestManager__
#define __Stonefish__ConsoleTestManager__

#include "SimulationManager.h"

class ConsoleTestManager : public SimulationManager
{
public:
    ConsoleTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif
