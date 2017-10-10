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
#include "DynamicThruster.h"
#include "Thruster.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) : SimulationManager(SimulationType::MARINE, UnitSystems::MKS, stepsPerSecond, DANTZIG, STANDARD)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, false, false, true);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Plastic", UnitSystem::Density(CGS, MKS, 1.5), 0.4);
    getMaterialManager()->CreateMaterial("Cork", UnitSystem::Density(CGS, MKS, 0.7), 0.2);
    getMaterialManager()->SetMaterialsInteraction("Plastic", "Plastic", 0.9, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Plastic", "Cork", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Cork", "Cork", 0.9, 0.7);
    
    ///////LOOKS///////////
    int yellow = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 0.6f, 0.2f), 0.2f, 0.f);
    int green = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 1.0f, 0.2f), 0.2f, 0.f);
	
    ////////OBJECTS
	Cylinder* hull = new Cylinder("Hull", 0.15, 1.5, getMaterialManager()->getMaterial("Cork"), yellow);
	Cylinder* hullB = new Cylinder("HullB", 0.15, 1.5, getMaterialManager()->getMaterial("Plastic"), yellow);
	
	//Vehicle body
	Compound* comp = new Compound("Compound", hullB, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
	comp->AddExternalPart(hull, btTransform(btQuaternion::getIdentity(), btVector3(0.35,0,-0.7)));
	comp->AddExternalPart(hull, btTransform(btQuaternion::getIdentity(), btVector3(-0.35,0,-0.7)));
	
	UnderwaterVehicle* vehicle = new UnderwaterVehicle("AUV", comp, btTransform(btQuaternion(0,0,0), btVector3(0,0,2)));
	AddSystemEntity(vehicle, btTransform::getIdentity());
	
	Box* link1A = new Box("Link1A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	Box* link2A = new Box("Link2A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	Box* link3A = new Box("Link3A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	
	Sphere* baseA = new Sphere("BaseA", 0.1f, getMaterialManager()->getMaterial("Cork"), green);
	Manipulator* manipA = new Manipulator("ArmA", 3, baseA, vehicle->getVehicleBody());
	manipA->AddRotLinkDH(link1A, btTransform(btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	manipA->AddRotLinkDH(link2A, btTransform(btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	manipA->AddRotLinkDH(link3A, btTransform(btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	AddSystemEntity(manipA, btTransform(btQuaternion(M_PI_2, 0, M_PI_2), btVector3(0.1,0.75,2.5)));
	
    std::string path = GetDataPath() + "sphere_R=1.obj";
    
    Thruster* th1 = new Thruster("TH1", 0.2, 0.01, 0.5, 0.05, 10, 1, path, 0.5, true, green);
    th1->AttachToSolid(vehicle->getVehicleBody(), 0, btTransform(btQuaternion::getIdentity(), btVector3(0,0.2,0.0)));
    th1->Setpoint(0.5);
    AddActuator(th1);
    
	/*Sphere* duct = new Sphere("Duct", 0.05, getMaterialManager()->getMaterial("Plastic"), green); 
	Box* prop = new Box("Propeller", btVector3(0.05,0.2,0.01), getMaterialManager()->getMaterial("Plastic"), green);
	Thruster* th = new Thruster("Thruster", vehicle, duct, prop, 0.2, 0.2);
	AddSystemEntity(th, btTransform(btQuaternion::getIdentity(), btVector3(0.0, 0.0, 1.65)));
	
	th->SetDesiredSpeed(10);
	
	Sphere* duct2 = new Sphere("Duct2", 0.05, getMaterialManager()->getMaterial("Plastic"), green); 
	Box* prop2 = new Box("Propeller2", btVector3(0.05,0.2,0.01), getMaterialManager()->getMaterial("Plastic"), green);
	Thruster* th2 = new Thruster("Thruster2", vehicle, duct2, prop2, 0.2, 0.2);
	AddSystemEntity(th2, btTransform(btQuaternion(M_PI_2,0,0), btVector3(0.0, -0.6, 1.65)));
	
	th2->SetDesiredSpeed(10);
	*/
    //Setup model paths
    /*std::string path = GetDataPath() + "sphere_R=1.obj";
	
    //Reference solid
	Polyhedron* hull1 = new Polyhedron("Solid", path, btScalar(0.5), getMaterialManager()->getMaterial("Cork"), green, true);
    //AddSolidEntity(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0.0,0.0,-2.0)));

    //Vehicle
    Polyhedron* hull2 = new Polyhedron("Solid", path, btScalar(0.5), getMaterialManager()->getMaterial("Concrete"), yellow, true);
    
    UnderwaterVehicle* rov = new UnderwaterVehicle("ROV");
    rov->AddExternalPart(hull2, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-1.0)));
    rov->AddExternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,-0.7,0)));
    rov->AddExternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,0.7,0)));
    //rov->AddInternalPart(hull1, btTransform(btQuaternion::getIdentity(), btVector3(0,0.0,1.0)));
    AddSystemEntity(rov, btTransform(btQuaternion(0.1,0,0), btVector3(0,0,0)));
	
	//Box* vehicle = new Box("Box", btVector3(1.,1.,1.), getMaterialManager()->getMaterial("Cork"), yellow);
	//AddSolidEntity(vehicle, btTransform(btQuaternion::getIdentity(), btVector3(0,0,1)));
	
    Box* link1 = new Box("Link1", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	Box* link2 = new Box("Link2", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	Box* link3 = new Box("Link3", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Cork"), green);
	
	Sphere* base = new Sphere("Sphere", 0.1f, getMaterialManager()->getMaterial("Cork"), green);
	Manipulator* manip = new Manipulator("Arm", 3, base, btTransform(btQuaternion::btQuaternion(btVector3(1,0,0), M_PI_2), btVector3(0,0,1)), rov->getRigidBody());
	manip->AddRotLinkDH(link1, btTransform(btQuaternion::getIdentity(), btVector3(-0.25f,0,0)), 0, 0.6f, 0);
	manip->AddRotLinkDH(link2, btTransform(btQuaternion::getIdentity(), btVector3(-0.25f,0,0)), 0, 0.6f, 0);
	manip->AddRotLinkDH(link3, btTransform(btQuaternion::getIdentity(), btVector3(-0.25f,0,0)), 0, 0.6f, 0);
	AddEntity(manip);*/
	
	//Box* box = new Box("Box", btVector3(1.,1.,1.), getMaterialManager()->getMaterial("Cork"), green);
	//AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
	
	//Sphere* sphere = new Sphere("Sphere", 0.5, getMaterialManager()->getMaterial("Cork"), green);
	//AddSolidEntity(sphere, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-2.)));
	
	//Cylinder* cylinder = new Cylinder("Cylinder", 0.5, 1., getMaterialManager()->getMaterial("Cork"), green);
	//AddSolidEntity(cylinder, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-4.)));
	
	//Torus* torus = new Torus("Torus", 1., 0.1, getMaterialManager()->getMaterial("Cork"), yellow);
	//AddSolidEntity(torus, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-6.)));
}
