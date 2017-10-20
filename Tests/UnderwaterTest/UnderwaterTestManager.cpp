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
#include "Compound.h"
#include "OpenGLPointLight.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "SystemUtil.hpp"
#include "Accelerometer.h"
#include "ADC.h"
#include "Ocean.h"
#include "Obstacle.h"
#include "UnderwaterVehicle.h"
#include "Thruster.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) : SimulationManager(SimulationType::POOL, UnitSystems::MKS, stepsPerSecond, DANTZIG)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(false, false, false, false, false, false, false);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Plastic", UnitSystem::Density(CGS, MKS, 1.0), 0.4);
    getMaterialManager()->CreateMaterial("Cork", UnitSystem::Density(CGS, MKS, 0.7), 0.2);
    getMaterialManager()->SetMaterialsInteraction("Plastic", "Plastic", 0.9, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Plastic", "Cork", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Cork", "Cork", 0.9, 0.7);
    
    ///////LOOKS///////////
    int yellow = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 0.9f, 0.f), 0.3f, 0.f);
    int green = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 1.0f, 0.2f), 0.2f, 0.f);
	int grey = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    int propLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 1.f, 1.f), 0.3f, 0.f, GetDataPath() + "propeller_tex.png");
    int ductLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 1.f, 1.f), 0.4f, 0.5f, GetDataPath() + "duct_tex.png");
    int manipLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    
    ////////OBJECTS    
	//Create underwater vehicle body
    Polyhedron* hullP = new Polyhedron("HullPort", GetDataPath() + "hull_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Cork"), yellow, false);
    Polyhedron* hullS = new Polyhedron("HullStarboard", GetDataPath() + "hull_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Cork"), yellow, false);
    Polyhedron* hullB = new Polyhedron("HullBottom", GetDataPath() + "hull_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), yellow, false);
    Polyhedron* vBarStern = new Polyhedron("VBarStern", GetDataPath() + "vbar_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), grey, false);
    Polyhedron* vBarBow = new Polyhedron("VBarBow", GetDataPath() + "vbar_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), grey, false);
    Polyhedron* ductSway = new Polyhedron("DuctSway", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), ductLook, false);
    Polyhedron* ductSurgeP = new Polyhedron("DuctSurgePort", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), ductLook, false);
    Polyhedron* ductSurgeS = new Polyhedron("DuctSurgeStarboard", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), ductLook, false);
    Polyhedron* ductHeaveS = new Polyhedron("DuctHeaveStern", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), ductLook, false);
    Polyhedron* ductHeaveB = new Polyhedron("DuctHeaveBow", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), ductLook, false);
    
    Compound* comp = new Compound("Compound", hullB, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)));
	comp->AddExternalPart(hullP, btTransform(btQuaternion(0,0,0), btVector3(0,-0.35,-0.7)));
	comp->AddExternalPart(hullS, btTransform(btQuaternion(0,0,0), btVector3(0,0.35,-0.7)));
    comp->AddExternalPart(vBarStern, btTransform(btQuaternion::getIdentity(), btVector3(-0.25,0.0,-0.15)));
    comp->AddExternalPart(vBarBow, btTransform(btQuaternion::getIdentity(), btVector3(0.30,0.0,-0.15)));
    comp->AddExternalPart(ductSway, btTransform(btQuaternion(M_PI_2,0,0), btVector3(-0.0137, -0.0307, -0.38)));
    comp->AddExternalPart(ductSurgeP, btTransform(btQuaternion(0,0,M_PI), btVector3(-0.2807,-0.2587,-0.38)));
    comp->AddExternalPart(ductSurgeS, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    comp->AddExternalPart(ductHeaveS, btTransform(btQuaternion(M_PI_2,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    comp->AddExternalPart(ductHeaveB, btTransform(btQuaternion(-M_PI_2,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
	
    //Create underwater vehicle
	UnderwaterVehicle* vehicle = new UnderwaterVehicle("AUV", comp);
	AddSystemEntity(vehicle, btTransform(btQuaternion(0,0,0), btVector3(0,0,-2)));
	
    //Create thrusters
    Polyhedron* propSway = new Polyhedron("PropellerSway", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), propLook, false);
    Thruster* thSway = new Thruster("ThrusterSway", propSway, 0.18, 0.48, 0.05, 100.0);
    thSway->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(M_PI_2,0,0), btVector3(-0.0137, -0.0307, -0.38)));
    thSway->setSetpoint(0.0);
    AddActuator(thSway);
    
    Polyhedron* propSurgeP = new Polyhedron("PropellerSurgePort", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), propLook, false);
    Thruster* thSurgeP = new Thruster("ThrusterSurgePort", propSurgeP, 0.18, 0.48, 0.05, 100.0);
    thSurgeP->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,-0.2587,-0.38)));
    thSurgeP->setSetpoint(1.0);
    AddActuator(thSurgeP);
    
    Polyhedron* propSurgeS = new Polyhedron("PropellerSurgeStarboard", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), propLook, false);
    Thruster* thSurgeS = new Thruster("ThrusterSurgeStarboard", propSurgeS, 0.18, 0.48, 0.05, 100.0);
    thSurgeS->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    thSurgeS->setSetpoint(0.0);
    AddActuator(thSurgeS);
    
    Polyhedron* propHeaveS = new Polyhedron("PropellerHeaveStern", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), propLook, false);
    Thruster* thHeaveS = new Thruster("ThrusterHeaveStern", propHeaveS, 0.18, 0.48, 0.05, 100.0);
    thHeaveS->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    thHeaveS->setSetpoint(0.0);
    AddActuator(thHeaveS);
    
    Polyhedron* propHeaveB = new Polyhedron("PropellerHeaveBow", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), propLook, false);
    Thruster* thHeaveB = new Thruster("ThrusterHeaveBow", propHeaveB, 0.18, 0.48, 0.05, 100.0);
    thHeaveB->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
    thHeaveB->setSetpoint(0.0);
    AddActuator(thHeaveB);
    
    //Create manipulator
    Polyhedron* baseLink = new Polyhedron("ArmBaseLink", GetDataPath() + "baseLink_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), manipLook, false);
    Polyhedron* baseLink2 = new Polyhedron("ArmBaseLink", GetDataPath() + "baseLink_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), manipLook, false);
    Polyhedron* baseLink3 = new Polyhedron("ArmBaseLink", GetDataPath() + "baseLink_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Plastic"), manipLook, false);
    
    Box* link1A = new Box("Link1A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	Box* link2A = new Box("Link2A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	Box* link3A = new Box("Link3A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	
	Manipulator* arm = new Manipulator("Arm", 2, baseLink, btTransform(btQuaternion::getIdentity(), btVector3(0.255,-0.02,0)), vehicle->getVehicleBody());
	arm->AddRotLinkDH(baseLink2, btTransform(btQuaternion(0,0,0), btVector3(-0.5,0,0)), 0, 1.0, 0.5*M_PI_2);
	arm->AddRotLinkDH(baseLink3, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), 0, 0.5, 0);
	//arm->AddRotLinkDH(link3A, btTransform(btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	AddSystemEntity(arm, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0.0,0.0,-1.5)));
}
