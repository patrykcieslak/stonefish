//
//  XMLSimulationManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/10/12.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "XMLSimulationManager.h"

XMLSimulationManager::XMLSimulationManager(std::string xmlPath, SimulationType t, UnitSystems unitSystem, btScalar stepsPerSecond, SolverType st, CollisionFilteringType cft, HydrodynamicsType ht)
    : SimulationManager(t, unitSystem, stepsPerSecond, st, cft, ht)
{
}
    
void XMLSimulationManager::BuildScenario()
{
    //Parse from XML
}