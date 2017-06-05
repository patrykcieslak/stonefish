//
//  UnderwaterTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestManager__
#define __Stonefish__UnderwaterTestManager__

#include "SimulationManager.h"

class UnderwaterTestManager : public SimulationManager
{
public:
    UnderwaterTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif
