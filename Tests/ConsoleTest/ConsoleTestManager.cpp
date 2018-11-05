//
//  ConsoleTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/09/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/solids/Sphere.h>

ConsoleTestManager::ConsoleTestManager(btScalar stepsPerSecond) 
    : SimulationManager(UnitSystems::MKS, true, stepsPerSecond, SolverType::SI, CollisionFilteringType::EXCLUSIVE, HydrodynamicsType::GEOMETRY_BASED)
{
}

void ConsoleTestManager::BuildScenario()
{
	//Create materials
	getMaterialManager()->CreateMaterial("Rock", UnitSystem::Density(CGS, MKS, 3.0), 0.8);
	getMaterialManager()->SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
	
	//Build scene	
	Plane* plane = new Plane("Bottom", 1000.0, getMaterialManager()->getMaterial("Rock"));
    AddStaticEntity(plane, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
	
	Sphere* sph = new Sphere("Sphere", 0.1, getMaterialManager()->getMaterial("Rock"));
	AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0,0,1.0)));
}

void ConsoleTestManager::SimulationStepCompleted()
{
	SimulationManager::SimulationStepCompleted();
	
    SolidEntity* ent = (SolidEntity*)getEntity("Sphere");
	cInfo("Sphere height: %1.3lf", ent->getTransform().getOrigin().getZ());
}