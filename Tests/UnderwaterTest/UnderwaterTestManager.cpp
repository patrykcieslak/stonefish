//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include "Plane.h"
#include "Polyhedron.h"
#include "Box.h"
#include "Sphere.h"
#include "Torus.h"
#include "Cylinder.h"
#include "OpenGLPointLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "SystemUtil.hpp"
#include "Accelerometer.h"
#include "ADC.h"
#include "Ocean.h"
#include "Obstacle.h"
#include "UnderwaterVehicle.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) : SimulationManager(SimulationType::MARINE, UnitSystems::MKS, stepsPerSecond, DANTZIG, STANDARD)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, true, false);
    //OpenGLPipeline::getInstance()->setDebugSimulation(true);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 1.1), 0.4);
    getMaterialManager()->CreateMaterial("Cork", UnitSystem::Density(CGS, MKS, 0.9), 0.2);
    getMaterialManager()->CreateFluid("Water", UnitSystem::Density(CGS, MKS, 1.0), 1.308e-3, 1.55);
    
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Cork", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Cork", "Cork", 0.9, 0.7);
    
    ///////LOOKS///////////
    int yellow = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 0.6f, 0.2f), 0.5f, 0.f);
    int green = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 1.0f, 0.2f), 0.5f, 0.f);
    
    ////////OBJECTS
    EnableOcean(getMaterialManager()->getFluid("Water"));
    
    //Setup model paths
    std::string path = GetDataPath() + "sphere_R=1.obj";
	
    //Reference solid
	Polyhedron* hull1 = new Polyhedron("Solid", path, btScalar(0.5), getMaterialManager()->getMaterial("Cork"), green, true);
    AddSolidEntity(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0.0,0.0,-2.0)));
/*
    //Vehicle
    Polyhedron* hull2 = new Polyhedron("Solid", path, btScalar(0.5), getMaterialManager()->getMaterial("Concrete"), yellow);
    
    UnderwaterVehicle* rov = new UnderwaterVehicle("ROV");
    rov->AddExternalPart(hull2, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-1.0)));
    rov->AddExternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,-0.7,0)));
    rov->AddExternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,0.7,0)));
    //rov->AddInternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,0.0,1.0)));
    AddSystemEntity(rov, btTransform(btQuaternion(0.1,0,0), btVector3(0,0,0)));
    */
	
	//////////////////////////////////////////////////////////////
	//OBLICZANIE AABB DLA UNDERWATERVEHICLE -> POPRAWNE CIENIE!!!!
	/////////////////////////////////////////
	
	//Box* box = new Box("Box", btVector3(1.,1.,1.), getMaterialManager()->getMaterial("Cork"), green);
	//AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
	
	//Sphere* sphere = new Sphere("Sphere", 0.5, getMaterialManager()->getMaterial("Cork"), green);
	//AddSolidEntity(sphere, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-2.)));
	
	Cylinder* cylinder = new Cylinder("Cylinder", 0.5, 1., getMaterialManager()->getMaterial("Cork"), green);
	AddSolidEntity(cylinder, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-4.)));
	
	Torus* torus = new Torus("Torus", 1., 0.1, getMaterialManager()->getMaterial("Cork"), yellow);
	AddSolidEntity(torus, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-6.)));
}
