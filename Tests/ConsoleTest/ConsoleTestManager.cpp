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
#include <core/Robot.h>

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
	//Plane* plane = new Plane("Bottom", 1000.0, getMaterialManager()->getMaterial("Rock"));
    //AddStaticEntity(plane, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
	
	Sphere* sph1 = new Sphere("Sphere1", 0.1, getMaterialManager()->getMaterial("Rock"));
	Sphere* sph2 = new Sphere("Sphere2", 0.1, getMaterialManager()->getMaterial("Rock"));
	
	std::vector<SolidEntity*> links(0);
	links.push_back(sph2);
	
	Robot* robot = new Robot("Robot", false);
	robot->DefineLinks(sph1, links);
	robot->DefineRevoluteJoint("Joint1", 
							   "Sphere1", 
							   "Sphere2", 
							   btTransform(btQuaternion::getIdentity(), btVector3(0.5,0,0.0)), 
							   btVector3(0.0,1.0,0.0), 
							   std::make_pair<btScalar,btScalar>(-1.0,1.0),
							   1.0);
							   
	robot->AddToDynamicsWorld(getDynamicsWorld(), btTransform::getIdentity());
}

void ConsoleTestManager::SimulationStepCompleted()
{
	SimulationManager::SimulationStepCompleted();
	
    //SolidEntity* ent = (SolidEntity*)getEntity("Sphere");
	//cInfo("Sphere height: %1.3lf", ent->getTransform().getOrigin().getZ());
}