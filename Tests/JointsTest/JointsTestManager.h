//
//  JointsTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__JointsTestManager__
#define __Stonefish__JointsTestManager__

#include <core/SimulationManager.h>

class JointsTestManager : public sf::SimulationManager
{
public:
    JointsTestManager(sf::Scalar stepsPerSecond);
    
    void BuildScenario();
};

#endif
