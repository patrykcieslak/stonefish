//
//  AcrobotTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestManager__
#define __Stonefish__AcrobotTestManager__

#include "SimulationManager.h"

#define USE_FEATHERSTONE_ALGORITHM

class AcrobotTestManager : public SimulationManager
{
public:
    AcrobotTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif