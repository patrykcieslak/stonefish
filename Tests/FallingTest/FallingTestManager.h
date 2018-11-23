//
//  FallingTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__FallingTestManager__
#define __Stonefish__FallingTestManager__

#include <core/SimulationManager.h>

class FallingTestManager : public sf::SimulationManager
{
public:
    FallingTestManager(btScalar stepsPerSecond);
    
    void BuildScenario();
};

#endif
