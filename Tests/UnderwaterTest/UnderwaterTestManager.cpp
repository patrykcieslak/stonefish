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
#include "Ocean.h"
#include "Obstacle.h"
#include "UnderwaterVehicle.h"
#include "Thruster.h"
#include "FixedGripper.h"
#include "Pressure.h"
#include "DVL.h"
#include "FOG.h"
#include "IMU.h"
#include "GPS.h"
#include "Contact.h"
#include "Camera.h"
#include "Light.h"
#include "FakeRotaryEncoder.h"
#include "Accelerometer.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) 
    : SimulationManager(SimulationType::POOL, UnitSystems::MKS, stepsPerSecond, SolverType::SI, CollisionFilteringType::EXCLUSIVE, HydrodynamicsType::GEOMETRY_BASED)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(false, false, false, false, false, false, false);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Dummy", UnitSystem::Density(CGS, MKS, 1.0), 0.5);
    getMaterialManager()->CreateMaterial("Fiberglass", UnitSystem::Density(CGS, MKS, 1.5), 0.3);
    getMaterialManager()->CreateMaterial("Rock", UnitSystem::Density(CGS, MKS, 3.0), 0.8);
    
    getMaterialManager()->SetMaterialsInteraction("Dummy", "Dummy", 0.5, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
    
    getMaterialManager()->SetMaterialsInteraction("Fiberglass", "Dummy", 0.5, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Rock", "Dummy", 0.6, 0.4);
    getMaterialManager()->SetMaterialsInteraction("Rock", "Fiberglass", 0.6, 0.4);
    
    ///////LOOKS///////////
    int yellow = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 0.9f, 0.f), 0.3f, 0.f);
    int grey = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    int seabed = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.7f, 0.7f, 0.5f), 0.9f, 0.f);
    int propLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 1.f, 1.f), 0.3f, 0.f, GetDataPath() + "propeller_tex.png");
    int ductLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.1f, 0.1f, 0.1f), 0.4f, 0.5f);
    int manipLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    int link4Look = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(1.f, 1.f, 1.f), 0.6f, 0.8f, GetDataPath() + "link4_tex.png");
    int eeLook = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.59f, 0.56f, 0.51f), 0.6f, 0.8f);
    
    ////////OBJECTS    
    //Create environment
    Plane* plane = new Plane("Bottom", 1000.0, getMaterialManager()->getMaterial("Rock"), seabed);
    AddStaticEntity(plane, btTransform(btQuaternion::getIdentity(), btVector3(0,0,7.0)));    
    
    for(unsigned int i=0; i<10; ++i)
    {
        Obstacle* cyl = new Obstacle("Rock", 1.0,3.0, getMaterialManager()->getMaterial("Rock"), seabed);
        AddStaticEntity(cyl, btTransform(btQuaternion::getIdentity(), btVector3(i*2.0,0,5.5)));    
    }
    
    //Box* box = new Box("Test", btVector3(1.0,1.0,0.5), getMaterialManager()->getMaterial("Rock"), seabed);
    //AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0,0,3.0)));
    
	//Create underwater vehicle body
    //Externals
    Polyhedron* hullB = new Polyhedron("HullBottom", GetDataPath() + "hull_hydro.obj", btScalar(1.0), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, btScalar(0.003), false);
    Polyhedron* hullP = new Polyhedron("HullPort", GetDataPath() + "hull_hydro.obj", btScalar(1.0), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, btScalar(0.003), false);
    Polyhedron* hullS = new Polyhedron("HullStarboard", GetDataPath() + "hull_hydro.obj", btScalar(1.0), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, btScalar(0.003), false);
    Polyhedron* vBarStern = new Polyhedron("VBarStern", GetDataPath() + "vbar_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), grey, false);
    Polyhedron* vBarBow = new Polyhedron("VBarBow", GetDataPath() + "vbar_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), grey, false);
    Polyhedron* ductSway = new Polyhedron("DuctSway", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductSurgeP = new Polyhedron("DuctSurgePort", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductSurgeS = new Polyhedron("DuctSurgeStarboard", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductHeaveS = new Polyhedron("DuctHeaveStern", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    Polyhedron* ductHeaveB = new Polyhedron("DuctHeaveBow", GetDataPath() + "duct_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    //Internals
    Cylinder* batteryCyl = new Cylinder("BatteryCylinder", 0.13, 0.6, getMaterialManager()->getMaterial("Dummy"), manipLook);
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(btScalar(47));
    Cylinder* portCyl = new Cylinder("PortCylinder", 0.13, 1.0, getMaterialManager()->getMaterial("Dummy"), manipLook);
    portCyl->ScalePhysicalPropertiesToArbitraryMass(btScalar(35));
    Cylinder* starboardCyl = new Cylinder("StarboardCylinder", 0.13, 1.0, getMaterialManager()->getMaterial("Dummy"), manipLook);
    starboardCyl->ScalePhysicalPropertiesToArbitraryMass(btScalar(35));
    
    //Build whole body
    Compound* comp = new Compound("Compound", hullB, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)));
	comp->AddExternalPart(hullP, btTransform(btQuaternion(0,0,0), btVector3(0,-0.35,-0.7)));
	comp->AddExternalPart(hullS, btTransform(btQuaternion(0,0,0), btVector3(0,0.35,-0.7)));
    comp->AddExternalPart(vBarStern, btTransform(btQuaternion::getIdentity(), btVector3(-0.25,0.0,-0.15)));
    comp->AddExternalPart(vBarBow, btTransform(btQuaternion::getIdentity(), btVector3(0.30,0.0,-0.15)));
    comp->AddExternalPart(ductSway, btTransform(btQuaternion(M_PI_2,M_PI,0), btVector3(-0.0137, 0.0307, -0.38)));
    comp->AddExternalPart(ductSurgeP, btTransform(btQuaternion(0,0,M_PI), btVector3(-0.2807,-0.2587,-0.38)));
    comp->AddExternalPart(ductSurgeS, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    comp->AddExternalPart(ductHeaveS, btTransform(btQuaternion(M_PI_2,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    comp->AddExternalPart(ductHeaveB, btTransform(btQuaternion(-M_PI_2,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
	comp->AddInternalPart(batteryCyl, btTransform(btQuaternion(M_PI_2,0,0), btVector3(-0.1,0,0)));
    comp->AddInternalPart(portCyl, btTransform(btQuaternion(M_PI_2,0,0), btVector3(0.0,-0.35,-0.7)));
    comp->AddInternalPart(starboardCyl, btTransform(btQuaternion(M_PI_2,0,0), btVector3(0.0,0.35,-0.7)));
    
    //Create underwater vehicle
    btScalar depth = 0.7;
    
	UnderwaterVehicle* vehicle = new UnderwaterVehicle("AUV", comp);
	AddSystemEntity(vehicle, btTransform(btQuaternion(0,0,0), btVector3(0,0,depth)));
    Accelerometer* acc = new Accelerometer("Acc", comp, btTransform::getIdentity(), -1, 0);
    AddSensor(acc);
    
    //Add sensors
    Pressure* press = vehicle->AddPressureSensor(btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), 1.0);
    press->SetNoise(1.0);
    DVL* dvl = vehicle->AddDVL(btTransform(btQuaternion(0,0,M_PI), btVector3(0,0,0)), UnitSystem::Angle(true, 30.0));
    dvl->SetNoise(0.02, 0.05);
    IMU* imu = vehicle->AddIMU(btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
    imu->SetNoise(0.01, 0.05);
    FOG* fog = vehicle->AddFOG(btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
    fog->SetNoise(0.001);
    GPS* gps = vehicle->AddGPS(btTransform(btQuaternion::getIdentity(), btVector3(0,0,-1)), UnitSystem::Angle(true, 50), UnitSystem::Angle(true, 20));
    gps->SetNoise(0.000001, 0.000001);

    //Create and attach thrusters
    Polyhedron* prop1 = new Polyhedron("Propeller", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Polyhedron* prop2 = new Polyhedron("Propeller", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Polyhedron* prop3 = new Polyhedron("Propeller", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Polyhedron* prop4 = new Polyhedron("Propeller", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Polyhedron* prop5 = new Polyhedron("Propeller", GetDataPath() + "propeller.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    Thruster* thSway = new Thruster("ThrusterSway", prop1, 0.18, 0.48, 0.05, 1000.0);
    Thruster* thSurgeP = new Thruster("ThrusterSurgePort", prop2, 0.18, 0.48, 0.05, 1000.0);
    Thruster* thSurgeS = new Thruster("ThrusterSurgeStarboard", prop3, 0.18, 0.48, 0.05, 1000.0);
    Thruster* thHeaveS = new Thruster("ThrusterHeaveStern", prop4, 0.18, 0.48, 0.05, 1000.0);
    Thruster* thHeaveB = new Thruster("ThrusterHeaveBow", prop5, 0.18, 0.48, 0.05, 1000.0);
    vehicle->AddThruster(thSway, btTransform(btQuaternion(M_PI_2,M_PI,0), btVector3(-0.0137, 0.0307, -0.38)));
    vehicle->AddThruster(thSurgeP, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,-0.2587,-0.38)));
    vehicle->AddThruster(thSurgeS, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    vehicle->AddThruster(thHeaveS, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    vehicle->AddThruster(thHeaveB, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
    
    FakeRotaryEncoder* enc = new FakeRotaryEncoder("Encoder", thSway, -1, 400);
    AddSensor(enc);
    
    //Create manipulator
    //-->Create solids
    Polyhedron* baseLink = new Polyhedron("ArmBaseLink", GetDataPath() + "base_link_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link1 = new Polyhedron("ArmLink1", GetDataPath() + "link1_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link2 = new Polyhedron("ArmLink2", GetDataPath() + "link2_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link3 = new Polyhedron("ArmLink3", GetDataPath() + "link3_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link4 = new Polyhedron("ArmLink4", GetDataPath() + "link4ft_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), link4Look, false);
    
    //-->Build manipulator
	Manipulator* arm = new Manipulator("Arm", 4, baseLink, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), vehicle->getVehicleBody());
    arm->AddRotLinkDH(link1, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), -0.0136, 0.1065, M_PI_2);
	arm->AddRotLinkDH(link2, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), 0, 0.23332, 0);
	arm->AddRotLinkDH(link3, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), 0, 0.103, -M_PI_2);
    arm->AddTransformDH(0.201,0,0);
    arm->AddRotLinkDH(link4, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), 0, 0, 0);
	AddSystemEntity(arm, btTransform(btQuaternion(0,0,0), btVector3(0.90,0.0,depth)));
    
    //Create gripper
    Polyhedron* eeBase = new Polyhedron("EEBase", GetDataPath() + "eeprobe_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), eeLook, false);
    Sphere* eeTip = new Sphere("EETip", 0.015, getMaterialManager()->getMaterial("Dummy"), eeLook);
    Compound* ee = new Compound("EndEffector", eeBase, btTransform::getIdentity());
    ee->AddExternalPart(eeTip, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0.124+0.05)));
    
    FixedGripper* gripper = new FixedGripper("Gripper", arm, ee);
    AddSystemEntity(gripper, btTransform(btQuaternion::getIdentity(), btVector3(0,0, 0.05)));
    
    //Add contact sensing between gripper and target
    Contact* cnt = AddContact(ee, plane, 10000);
    cnt->setDisplayMask(CONTACT_DISPLAY_PATH_B);
    
    //Camera
    //Camera* cam = new Camera("Camera", 600, 400, 90.0, btTransform(btQuaternion(0,-0.1,M_PI), btVector3(0.5,0.0,-0.35)), comp, -1.0, 1, true);
    //AddSensor(cam);
    
    //Light
    //Light* l = new Light("Spot", btVector3(0,0,0), btVector3(0,0,-1), 30.0, OpenGLLight::ColorFromTemperature(4500, 1000000));
    //AddActuator(l);
}
