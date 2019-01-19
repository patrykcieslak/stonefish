//
//  AcrobotTestManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestManager__
#define __Stonefish__AcrobotTestManager__

#include <core/SimulationManager.h>

#define USE_FEATHERSTONE_ALGORITHM

class AcrobotTestManager : public sf::SimulationManager
{
public:
    AcrobotTestManager(sf::Scalar stepsPerSecond);
    
    void BuildScenario();
};

#endif
