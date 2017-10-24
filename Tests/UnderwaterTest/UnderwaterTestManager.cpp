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
#include "DCMotor.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) 
    : SimulationManager(SimulationType::POOL, UnitSystems::MKS, stepsPerSecond, SolverType::DANTZIG, CollisionFilteringType::EXCLUSIVE, HydrodynamicsType::GEOMETRY_BASED)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, false, false, false);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Dummy", UnitSystem::Density(CGS, MKS, 1.0), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Dummy", "Dummy", 0.5, 0.2);
    
    ///////LOOKS///////////
    int yellow = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 0.9f, 0.f), 0.3f, 0.f);
    int grey = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    int propLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 1.f, 1.f), 0.3f, 0.f, GetDataPath() + "propeller_tex.png");
    int ductLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 1.f, 1.f), 0.4f, 0.5f, GetDataPath() + "duct_tex.png");
    int manipLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    
    ////////OBJECTS    
	//Create underwater vehicle body
    Polyhedron* hullP = new Polyhedron("HullPort", GetDataPath() + "hull_hydro.obj", btScalar(1.0), getMaterialManager()->getMaterial("Dummy"), yellow, false);
    hullP->ScalePhysicalPropertiesToArbitraryMass(btScalar(40));
    Polyhedron* hullS = new Polyhedron("HullStarboard", GetDataPath() + "hull_hydro.obj", btScalar(1.0), getMaterialManager()->getMaterial("Dummy"), yellow, false);
    hullS->ScalePhysicalPropertiesToArbitraryMass(btScalar(40));
    Polyhedron* hullB = new Polyhedron("HullBottom", GetDataPath() + "hull_hydro.obj", btScalar(1.0), getMaterialManager()->getMaterial("Dummy"), yellow, false);
    hullB->ScalePhysicalPropertiesToArbitraryMass(btScalar(100));
    
    Polyhedron* vBarStern = new Polyhedron("VBarStern", GetDataPath() + "vbar_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), grey, false);
    Polyhedron* vBarBow = new Polyhedron("VBarBow", GetDataPath() + "vbar_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), grey, false);
    Polyhedron* ductSway = new Polyhedron("DuctSway", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductSurgeP = new Polyhedron("DuctSurgePort", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductSurgeS = new Polyhedron("DuctSurgeStarboard", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductHeaveS = new Polyhedron("DuctHeaveStern", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductHeaveB = new Polyhedron("DuctHeaveBow", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    
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
	AddSystemEntity(vehicle, btTransform(btQuaternion(0,0,0), btVector3(0,0,2)));
	
    //Create thrusters and attach to vehicle
    Polyhedron* propSway = new Polyhedron("PropellerSway", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Thruster* thSway = new Thruster("ThrusterSway", propSway, 0.18, 0.48, 0.05, 100.0);
    thSway->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(M_PI_2,0,0), btVector3(-0.0137, -0.0307, -0.38)));
    thSway->setSetpoint(0.0);
    AddActuator(thSway);
    
    Polyhedron* propSurgeP = new Polyhedron("PropellerSurgePort", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Thruster* thSurgeP = new Thruster("ThrusterSurgePort", propSurgeP, 0.18, 0.48, 0.05, 100.0);
    thSurgeP->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,-0.2587,-0.38)));
    thSurgeP->setSetpoint(0.0);
    AddActuator(thSurgeP);
    
    Polyhedron* propSurgeS = new Polyhedron("PropellerSurgeStarboard", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Thruster* thSurgeS = new Thruster("ThrusterSurgeStarboard", propSurgeS, 0.18, 0.48, 0.05, 100.0);
    thSurgeS->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    thSurgeS->setSetpoint(0.0);
    AddActuator(thSurgeS);
    
    Polyhedron* propHeaveS = new Polyhedron("PropellerHeaveStern", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Thruster* thHeaveS = new Thruster("ThrusterHeaveStern", propHeaveS, 0.18, 0.48, 0.05, 100.0);
    thHeaveS->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    thHeaveS->setSetpoint(0.0);
    AddActuator(thHeaveS);
    
    Polyhedron* propHeaveB = new Polyhedron("PropellerHeaveBow", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Thruster* thHeaveB = new Thruster("ThrusterHeaveBow", propHeaveB, 0.18, 0.48, 0.05, 100.0);
    thHeaveB->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
    thHeaveB->setSetpoint(0.0);
    AddActuator(thHeaveB);
    
    //Create manipulator
    //-->Create solids
    Polyhedron* baseLink = new Polyhedron("ArmBaseLink", GetDataPath() + "base_link_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link1 = new Polyhedron("ArmLink1", GetDataPath() + "link1_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link2 = new Polyhedron("ArmLink2", GetDataPath() + "link2_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link3 = new Polyhedron("ArmLink3", GetDataPath() + "link3_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link4 = new Polyhedron("ArmLink4", GetDataPath() + "link4_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    link4->ScalePhysicalPropertiesToArbitraryMass(1.0);
    
    //-->Create motors
    DCMotor* mot1 = new DCMotor("ArmMotor1", 1.0, 0.001, 1.0/200.0, 0.04, 0);
    mot1->SetupGearbox(true, 400.0, 0.8);
    DCMotor* mot2 = new DCMotor("ArmMotor2", 1.0, 0.001, 1.0/200.0, 0.04, 0);
    mot2->SetupGearbox(true, 400.0, 0.8);
    DCMotor* mot3 = new DCMotor("ArmMotor3", 1.0, 0.001, 1.0/400.0, 0.04, 0);
    mot3->SetupGearbox(true, 200.0, 0.8);
    DCMotor* mot4 = new DCMotor("ArmMotor4", 1.0, 0.001, 1.0/400.0, 0.01, 0);
    mot4->SetupGearbox(true, 20.0, 0.8);
    
    //-->Build manipulator
	Manipulator* arm = new Manipulator("Arm", 4, baseLink, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), vehicle->getVehicleBody());
    arm->AddRotLinkDH(link1, mot1, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), -0.0136, 0.1065, M_PI_2);
	arm->AddRotLinkDH(link2, mot2, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), 0, 0.23332, 0);
	arm->AddRotLinkDH(link3, mot3, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), 0, 0.103, -M_PI_2);
    arm->AddTransformDH(0.196,0,0);
    arm->AddRotLinkDH(link4, mot4, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), 0.006, 0, 0);
    
	AddSystemEntity(arm, btTransform(btQuaternion(0,0,0), btVector3(0.90,0.0,2)));
}
