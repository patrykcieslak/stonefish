//
//  XMLSimulationManager.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/10/12.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_XMLSimulationManager__
#define __Stonefish_XMLSimulationManager__

#include "SimulationManager.h"

class XMLSimulationManager : public SimulationManager
{
public:
    XMLSimulationManager(std::string xmlPath, SimulationType t, UnitSystems unitSystem, btScalar stepsPerSecond, SolverType st = SI, CollisionFilteringType cft = STANDARD, HydrodynamicsType ht = HydrodynamicsType::GEOMETRY_BASED);
    
    void BuildScenario();

private:
    
};

#endif