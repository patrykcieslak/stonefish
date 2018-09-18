//
//  ConsoleTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/09/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestManager.h"
#include "ConsoleSimulationApp.h"

ConsoleTestManager::ConsoleTestManager(btScalar stepsPerSecond) 
    : SimulationManager(UnitSystems::MKS, false, stepsPerSecond, SolverType::SI, CollisionFilteringType::EXCLUSIVE, HydrodynamicsType::GEOMETRY_BASED)
{
}

void ConsoleTestManager::BuildScenario()
{
}
