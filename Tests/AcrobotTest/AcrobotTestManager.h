//
//  AcrobotTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestManager__
#define __Stonefish__AcrobotTestManager__

#include "SimulationManager.h"

class AcrobotTestManager : public SimulationManager
{
public:
    AcrobotTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif