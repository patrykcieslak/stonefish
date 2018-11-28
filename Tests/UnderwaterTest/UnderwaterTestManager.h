//
//  UnderwaterTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestManager__
#define __Stonefish__UnderwaterTestManager__

#include <core/SimulationManager.h>

class UnderwaterTestManager : public sf::SimulationManager
{
public:
    UnderwaterTestManager(sf::Scalar stepsPerSecond);
    
    void BuildScenario();
    void SimulationStepCompleted();
};

#endif
