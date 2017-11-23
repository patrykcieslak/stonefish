//
//  FallingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "FallingTestManager.h"

#include "FallingTestApp.h"
#include "Plane.h"
#include "Obstacle.h"
#include "Polyhedron.h"
#include "Box.h"
#include "Sphere.h"
#include "Torus.h"
#include "Cylinder.h"
#include "OpenGLContent.h"
#include "SystemUtil.hpp"
#include "IMU.h"
#include "Trajectory.h"
#include "Manipulator.h"
#include "Light.h"
#include "Camera.h"
#include "UnderwaterVehicle.h"
#include "Thruster.h"
#include "Compound.h"
#include "Accelerometer.h"

FallingTestManager::FallingTestManager(btScalar stepsPerSecond) : SimulationManager(SimulationType::TERRESTIAL, UnitSystems::MKS, stepsPerSecond, SI, EXCLUSIVE)
{
}

void FallingTestManager::BuildScenario()
{
	OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, true, false, false);
	OpenGLPipeline::getInstance()->setDebugSimulation(false);
    setICSolverParams(false);
	
    ///////MATERIALS////////
	getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MKS, 1.0), 0.1);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);
    
	///////LOOKS///////////
    int yellow = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 0.6f, 0.2f), 0.2f, 0.f);
    int green = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 1.0f, 0.2f), 0.2f, 0.f);
	
    ////////OBJECTS
    //Setup model paths
    std::string path = GetDataPath() + "torpedo.stl";
	
    //Parts of vehicle body
	//Polyhedron* hull = new Polyhedron("Solid", path, btScalar(0.001), getMaterialManager()->getMaterial("Steel"), yellow, true);
    //Cylinder* hull = new Cylinder("Hull", 0.15, 1.5, getMaterialManager()->getMaterial("Steel"), yellow, -1, true);
	
	//Vehicle body
	//Compound* comp = new Compound("Compound", hull, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
	//comp->AddExternalPart(hull, btTransform(btQuaternion::getIdentity(), btVector3(0.35,0,0.7)));
	//comp->AddExternalPart(hull, btTransform(btQuaternion::getIdentity(), btVector3(-0.35,0,0.7)));
    //AddSolidEntity(comp, btTransform::getIdentity());
	
	/*UnderwaterVehicle* vehicle = new UnderwaterVehicle("AUV", comp, btTransform(btQuaternion(0,0,0), btVector3(0,0,2)));
	AddSystemEntity(vehicle, btTransform::getIdentity());
	
	Torus* duct = new Torus("Duct", 0.15,0.02, getMaterialManager()->getMaterial("Steel"), green); 
	Box* prop = new Box("Propeller", btVector3(0.05,0.2,0.01), getMaterialManager()->getMaterial("Steel"), green);
	Thruster* th = new Thruster("Thruster", vehicle, duct, prop, btTransform(btQuaternion::getIdentity(), btVector3(0.0, 0.0, 2.35)), 0.5, 1.0);
	AddSystemEntity(th, btTransform::getIdentity());
	
	th->SetDesiredSpeed(5000);
	*/
	
    Box* link1A = new Box("Link1A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Steel"), green);
	Box* link2A = new Box("Link2A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Steel"), green);
	Box* link3A = new Box("Link3A", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Steel"), green);
	
	Sphere* sph1 = new Sphere("Sphere1", 0.25f, getMaterialManager()->getMaterial("Steel"), green);
    Cylinder* cyl = new Cylinder("Cylinder1", 0.5f, 0.5f, getMaterialManager()->getMaterial("Steel"), green);
    Compound* baseA = new Compound("BaseA", sph1, btTransform::getIdentity());
    baseA->AddExternalPart(cyl, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.5)));
    
    AddSolidEntity(sph1, btTransform(btQuaternion::getIdentity(), btVector3(0,0,2)));
	
    Obstacle* obs = new Obstacle("Obs1", 0.25f, getMaterialManager()->getMaterial("Steel"), yellow);
    //AddStaticEntity(obs, btTransform::getIdentity());
    Obstacle* obs2 = new Obstacle("Obs2", 0.25f, getMaterialManager()->getMaterial("Steel"), yellow);
    //AddStaticEntity(obs2, btTransform(btQuaternion::getIdentity(), btVector3(2,0,0)));
    
	Sphere* sph2 = new Sphere("Sphere2", 0.25f, getMaterialManager()->getMaterial("Steel"), green);
	Compound* baseB = new Compound("BaseB", sph2, btTransform::getIdentity());
    FeatherstoneEntity* fe = new FeatherstoneEntity("Test", 1, baseB, getDynamicsWorld(), false);
	AddFeatherstoneEntity(fe, btTransform(btQuaternion::getIdentity(), btVector3(2,0,2)));
	
	//Manipulator* manipA = new Manipulator("ArmA", 1, baseA, btTransform::getIdentity());
	//manipA->AddRotLinkDH(link1A, btTransform(btQuaternion::btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	//manipA->AddRotLinkDH(link2A, btTransform(btQuaternion::btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	//manipA->AddRotLinkDH(link3A, btTransform(btQuaternion::btQuaternion(0,0,0), btVector3(-0.25,0,0)), 0, 0.5, 0);
	//AddSystemEntity(manipA, btTransform(btQuaternion(M_PI_2, 0, M_PI_2), btVector3(0.1,0.75,1.5)));
	
	Accelerometer* acc = new Accelerometer("Acc", baseB, btTransform::getIdentity(), -1.0, 0);
	AddSensor(acc);
	
	////////////////////////////// EVERYTHING ///////////////////////////////////
	
    ///////LOOKS///////////
	//std::string path = GetDataPath() + "grid.png";
    //int grid = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(1.f, 1.f, 1.f), 0.f, 0.1f, path);
            
    //Plane* floor = new Plane("Floor", 1000.f, getMaterialManager()->getMaterial("Ground"), btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), grid);
    //AddEntity(floor);
	
	/*int color = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(1.f, 0.5f, 0.1f), 0.1f, 0.1f);
    
	////////OBJECTS
	Sphere* vehicle = new Sphere("Vehicle", 1., getMaterialManager()->getMaterial("Steel"), color);
	AddSolidEntity(vehicle, btTransform(btQuaternion::getIdentity(), btVector3(0,0,1)));*/
	/*
    Sphere* sphere = new Sphere("Sphere", 0.1f, getMaterialManager()->getMaterial("Steel"), color);
	Box* link1 = new Box("Link1", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Steel"), color);
	Box* link2 = new Box("Link2", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Steel"), color);
	Box* link3 = new Box("Link3", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Steel"), color);
	
	Manipulator* manip = new Manipulator("Arm", 3, sphere, btTransform(btQuaternion::btQuaternion(btVector3(1,0,0), M_PI_2), btVector3(0,0,5)), vehicle->getRigidBody());
	manip->AddRotLinkDH(link1, btTransform(btQuaternion::getIdentity(), btVector3(-0.25f,0,0)), 0, 0.6f, 0);
	manip->AddRotLinkDH(link2, btTransform(btQuaternion::getIdentity(), btVector3(-0.25f,0,0)), 0, 0.6f, 0);
	manip->AddRotLinkDH(link3, btTransform(btQuaternion::getIdentity(), btVector3(-0.25f,0,0)), 0, 0.6f, 0);
	AddEntity(manip);*/
	
    /*for(int i=0; i<=10; ++i)
    {
        int lookId = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(i/10.f,1.f-i/10.f,0.f), i*0.05f+0.05f, 0.f);
        Sphere* sph = new Sphere("Sph" + std::to_string(i), 0.25, getMaterialManager()->getMaterial("Steel"), lookId);
        AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0., 0.0, 1.0+i/1.5)));
    }*/
	
	/*int lookId = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.0f,1.0f,1.0f), 0.1f, 0.0f);
	path = GetDataPath() + "dragon.obj";
	Polyhedron* poly = new Polyhedron("Dragon", path, 0.5f, getMaterialManager()->getMaterial("Steel"), lookId, false);
	btQuaternion quat;
	quat.setEulerZYX(0,-M_PI_2,0);
	AddSolidEntity(poly, btTransform(quat, btVector3(0,0,1.0)));*/
	
	
	/*for(int i=0; i<10; ++i)
	{
		Box* box = new Box("Box" + std::to_string(i), btVector3(0.5,0.5,5.0), getMaterialManager()->getMaterial("Steel"), lookId);
		AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0, i*4.0, 2.5)));				
	}*/
	
	/*for(int i=0; i<5; ++i)
		for(int h=0; h<5; ++h)
		{
			Box* box = new Box("Box" + std::to_string(i*10+h), btVector3(0.5,0.5,0.5), getMaterialManager()->getMaterial("Steel"), lookId);
			AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(-5.0+h, sinf(h/M_PI), i*0.6+0.5)));				
		}*/
    
    ////////SENSORS AND ACTUATORS
	//FakeIMU* imu = new FakeIMU("IMU", vehicle, btTransform::getIdentity());
	//AddSensor(imu);
	
    //Light* omni = new Light("Omni", btVector3(-2,2,2), OpenGLLight::ColorFromTemperature(4500, 1000000));
    //AddActuator(omni);
    //Light* spot = new Light("Spot", btVector3(5.f, 5.f, 5.f), btVector3(-1.f,-1.f,-1.f), 30.f, OpenGLLight::ColorFromTemperature(4500, 2000000));
    //AddActuator(spot);
    //Camera* cam = new Camera("Camera", btVector3(10,0,10000000), btVector3(0,0,0), btVector3(0,0,1.), 0, 0, FallingTestApp::getApp()->getWindowWidth()/3, FallingTestApp::getApp()->getWindowHeight()/3, 60.f);
    //AddSensor(cam);
	//OpenGLAtmosphere::getInstance()->SetSunPosition(0,-10.f);
}