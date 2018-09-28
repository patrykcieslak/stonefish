//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2018 Patryk Cieslak. All rights reserved.
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
#include "Odometry.h"
#include "DVL.h"
#include "FOG.h"
#include "IMU.h"
#include "GPS.h"
#include "Contact.h"
#include "ColorCamera.h"
#include "DepthCamera.h"
#include "Light.h"
#include "FakeRotaryEncoder.h"
#include "Accelerometer.h"
#include "FeatherstoneEntity.h"
#include "Trigger.h"
#include "TwoFingerGripper.h"
#include "Pipe.h"
#include "Jet.h"
#include "Profiler.h"
#include "Multibeam.h"

UnderwaterTestManager::UnderwaterTestManager(btScalar stepsPerSecond) 
    : SimulationManager(UnitSystems::MKS, false, stepsPerSecond, SolverType::SI, CollisionFilteringType::EXCLUSIVE, HydrodynamicsType::GEOMETRY_BASED)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, true, true, true, false, false, true);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    //getTrackball()->setEnabled(false);
    
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
	EnableOcean();
	
	Plane* plane = new Plane("Bottom", 1000.0, getMaterialManager()->getMaterial("Rock"), seabed);
    AddStaticEntity(plane, btTransform(btQuaternion::getIdentity(), btVector3(0,0,7.0)));    
	
    Pipe* vf = new Pipe(btVector3(10.0, 0.0, 1.0), btVector3(-10.0, 0.0, 1.0), 1.0, 1.0, 1.0, 1.0);
    //getOcean()->AddVelocityField(vf);
    
    Jet* vf2 = new Jet(btVector3(0.0, -3.0, 1.0),btVector3(0,1.0,0), 0.1, 10.0);
    //getOcean()->AddVelocityField(vf2);
    
	//Obstacle* bedrock = new Obstacle("Bedrock", GetDataPath() + "canyon.obj", 1.0, getMaterialManager()->getMaterial("Rock"), grey, false);
	//AddStaticEntity(bedrock, btTransform(btQuaternion(0.0,0.0,-M_PI_2), btVector3(0,0,7.0)));
    
    //std::vector<StaticEntity*> group;
    
    for(unsigned int i=0; i<10; ++i)
    {
        Obstacle* cyl = new Obstacle("Rock", 1.0,3.0, getMaterialManager()->getMaterial("Rock"), seabed);
        AddStaticEntity(cyl, btTransform(btQuaternion::getIdentity(), btVector3(i*2.0,0,5.5)));    
        //group.push_back(cyl);
    }
    /*
    StaticEntity::GroupTransform(group, btTransform(btQuaternion::getIdentity(), btVector3(9.0,0.0,0.0)), btTransform(btQuaternion(M_PI_2,0,0), btVector3(-4.0,0.0,0.0)));
        
    Box* box = new Box("Test", btVector3(1.0,1.0,0.5), getMaterialManager()->getMaterial("Rock"), propLook);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0,0,3.0)));*/
    
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
    
    //Manipulator bodies
    Polyhedron* baseLink = new Polyhedron("ArmBaseLink", GetDataPath() + "base_link_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    Polyhedron* link1 = new Polyhedron("ArmLink1", GetDataPath() + "link1_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true, HYDRO_PROXY_CYLINDER);
    Polyhedron* link2 = new Polyhedron("ArmLink2", GetDataPath() + "link2_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true, HYDRO_PROXY_CYLINDER);
    Polyhedron* link3 = new Polyhedron("ArmLink3", GetDataPath() + "link3_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true);//, HYDRO_PROXY_CYLINDER);
    Polyhedron* link4 = new Polyhedron("ArmLink4", GetDataPath() + "link4ft_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), link4Look, false, -1, true);//, HYDRO_PROXY_CYLINDER);
    
    //Create thrusters
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
    
    //Create gripper body
    Polyhedron* eeBase = new Polyhedron("EEBase", GetDataPath() + "eeprobe_hydro.obj", btScalar(1), getMaterialManager()->getMaterial("Dummy"), eeLook, false);
    Sphere* eeTip = new Sphere("EETip", 0.015, getMaterialManager()->getMaterial("Dummy"), eeLook);
    Compound* ee = new Compound("EE", eeBase, btTransform::getIdentity());
    ee->AddExternalPart(eeTip, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0.124)));

    //Create underwater vehicle
#ifdef USE_IAUV_CLASSES
    btScalar depth = 0.7;
    
	UnderwaterVehicle* vehicle = new UnderwaterVehicle("AUV", comp);
	AddSystemEntity(vehicle, btTransform(btQuaternion(0,0,0), btVector3(0,0,depth)));
    Accelerometer* acc = new Accelerometer("Acc", comp, btTransform::getIdentity(), -1, 0);
    AddSensor(acc);
    
    //Add sensors
	Odometry* odom = vehicle->AddOdometry(btTransform::getIdentity());
    Pressure* press = vehicle->AddPressureSensor(btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), 1.0);
    press->SetNoise(1.0);
    DVL* dvl = vehicle->AddDVL(btTransform(btQuaternion(0,0,M_PI), btVector3(0,0,0)), UnitSystem::Angle(true, 30.0));
    dvl->SetNoise(0.02, 0.05);
    IMU* imu = vehicle->AddIMU(btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
    imu->SetNoise(0.01, 0.05);
    FOG* fog = vehicle->AddFOG(btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)));
    fog->SetNoise(0.001);
    GPS* gps = vehicle->AddGPS(btTransform(btQuaternion::getIdentity(), btVector3(0,0,-1)), 41.77737, 3.03376);
    gps->SetNoise(0.5);

    //Attach thrusters
    vehicle->AddThruster(thSway, btTransform(btQuaternion(M_PI_2,M_PI,0), btVector3(-0.0137, 0.0307, -0.38)));
    vehicle->AddThruster(thSurgeP, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,-0.2587,-0.38)));
    vehicle->AddThruster(thSurgeS, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    vehicle->AddThruster(thHeaveS, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    vehicle->AddThruster(thHeaveB, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
    
    FakeRotaryEncoder* enc = new FakeRotaryEncoder("Encoder", thSway, -1, 400);
    AddSensor(enc);
    
    //Create manipulator
    Manipulator* arm = new Manipulator("Arm", 4, baseLink, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), vehicle->getVehicleBody());
    arm->AddRotLinkDH("Link1", link1, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), -0.0136, 0.1065, M_PI_2);
	arm->AddRotLinkDH("Link2", link2, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), 0, 0.23332, 0);
	arm->AddRotLinkDH("Link3", link3, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0)), 0, 0.103, -M_PI_2);
    arm->AddTransformDH(0.201,0,0);
    arm->AddRotLinkDH("Link4", link4, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)), 0, 0, 0);
	AddSystemEntity(arm, btTransform(btQuaternion(0,0,0), btVector3(0.90,0.0,depth)));
    
    //Add end-effector with force sensor
    //FixedGripper* gripper = new FixedGripper("Gripper", arm, ee);
    Box* eeBase0 = new Box("EEBase", btVector3(0.02,0.02,0.02), getMaterialManager()->getMaterial("Dummy"), manipLook);
    Box* eeFinger1_ = new Box("EEFinger1_", btVector3(0.02,0.1,0.2), getMaterialManager()->getMaterial("Dummy"), manipLook);
    Box* eeFinger2_ = new Box("EEFinger2_", btVector3(0.02,0.1,0.2), getMaterialManager()->getMaterial("Dummy"), manipLook);
    Compound* eeFinger1 = new Compound("EEFinger1", eeFinger1_, btTransform(btQuaternion::getIdentity(), btVector3(0.04, 0.0, 0.1)));
    Compound* eeFinger2 = new Compound("EEFinger2", eeFinger2_, btTransform(btQuaternion::getIdentity(), btVector3(-0.04, 0.0, 0.1)));
    
    TwoFingerGripper* gripper = new TwoFingerGripper("Gripper", arm, eeBase0, eeFinger1, eeFinger2, btVector3(0.04, 0.0, 0.0), btVector3(-0.04, 0.0, 0.0), btVector3(0.0, 1.0, 0.0), 0.3, 10.0);
    AddSystemEntity(gripper, btTransform(btQuaternion::getIdentity(), btVector3(0,0, 0.4)));
    
    //Add contact sensing between gripper and target
    Contact* cnt = AddContact(ee, plane, 10000);
    cnt->setDisplayMask(CONTACT_DISPLAY_PATH_B);
#else
    comp->AddExternalPart(baseLink, btTransform(btQuaternion::getIdentity(), btVector3(0.74, 0.0, 0.0)));
    
    //Build robot rigid body tree
    FeatherstoneEntity* iauv = new FeatherstoneEntity("IAUV", 5, comp, getDynamicsWorld(), false);
    iauv->setSelfCollision(true);
    iauv->AddLink(link1, btTransform(btQuaternion::getIdentity(), btVector3(0.74, 0.0, 0.0)), getDynamicsWorld());
    iauv->AddLink(link2, btTransform(btQuaternion::getIdentity(), btVector3(0.74 + 0.1065, 0.0, 0.0)), getDynamicsWorld());
    iauv->AddLink(link3, btTransform(btQuaternion::getIdentity(), btVector3(0.74 + 0.1065 + 0.23332, 0.0, 0.0)), getDynamicsWorld());
    iauv->AddLink(link4, btTransform(btQuaternion::getIdentity(), btVector3(0.74 + 0.1065 + 0.23332 + 0.103, 0.0, 0.201)), getDynamicsWorld());
    iauv->AddRevoluteJoint(0, 1, btVector3(0.74, 0.0, 0.0), btVector3(0.0,0.0,1.0));
    iauv->AddRevoluteJoint(1, 2, btVector3(0.74 + 0.1065, 0.0, 0.0), btVector3(0.0,1.0,0.0));
    iauv->AddRevoluteJoint(2, 3, btVector3(0.74 + 0.1065 + 0.23332, 0.0, 0.0), btVector3(0.0,1.0,0.0));
    iauv->AddRevoluteJoint(3, 4, btVector3(0.74 + 0.1065 + 0.23332 + 0.103, 0.0, 0.201), btVector3(0.0,0.0,1.0));
    AddFeatherstoneEntity(iauv, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)));
    
    //Add end-effector with force sensor
    FeatherstoneEntity* effector = new FeatherstoneEntity("EndEffector", 1, ee, getDynamicsWorld(), false);
    AddFeatherstoneEntity(effector, btTransform(btQuaternion::getIdentity(), btVector3(0.74 + 0.1065 + 0.23332 + 0.103, 0.0, 0.201+0.05)));
    FixedJoint* ftFix = new FixedJoint("Fix", effector, iauv, -1, 3);
    AddJoint(ftFix);
    ForceTorque* ft = new ForceTorque("FT", ftFix, effector->getLink(0).solid, btTransform::getIdentity());
    AddSensor(ft);
    
    //Add sensors
    Pressure* press = new Pressure("Pressure", comp, btTransform::getIdentity());
    AddSensor(press);
    DVL* dvl = new DVL("DVL", comp, btTransform(btQuaternion(0,0,M_PI), btVector3(0,0,0)), UnitSystem::Angle(true, 30.0));
    AddSensor(dvl);
    IMU* imu = new IMU("IMU", comp, btTransform::getIdentity());
    AddSensor(imu);
    FOG* fog = new FOG("FOG", comp, btTransform::getIdentity());
    AddSensor(fog);
    GPS* gps = new GPS("GPS", 20.0, 0.0, comp, btTransform::getIdentity());
    AddSensor(gps);
    
    //Attach thrusters
    thSway->AttachToSolid(iauv, 0, btTransform(btQuaternion(M_PI_2,M_PI,0), btVector3(-0.0137, 0.0307, -0.38)));
    thSurgeP->AttachToSolid(iauv, 0, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,-0.2587,-0.38)));
    thSurgeS->AttachToSolid(iauv, 0, btTransform(btQuaternion(0,0,0), btVector3(-0.2807,0.2587,-0.38)));
    thHeaveS->AttachToSolid(iauv, 0, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(-0.5337,0.0,-0.6747)));
    thHeaveB->AttachToSolid(iauv, 0, btTransform(btQuaternion(0,-M_PI_2,0), btVector3(0.5837,0.0,-0.6747)));
    AddActuator(thSway);
    AddActuator(thSurgeP);
    AddActuator(thSurgeS);
    AddActuator(thHeaveS);
    AddActuator(thHeaveB);
    
#endif
    
    //Profiler* prof = new Profiler("Laser", comp, btTransform(btQuaternion(0,0,0), btVector3(0,0,0.5)), 50.0, 100, 100.0);
    //AddSensor(prof);
    //Multibeam* mb = new Multibeam("Multibeam", comp, btTransform(btQuaternion(0,0,0), btVector3(0,0,0.5)), 120.0, 400, 10.0);
    //mb->SetRange(0.2, 10.0);
    //AddSensor(mb);
    
    //ColorCamera* cam = new ColorCamera("Camera", 600, 400, 90.0, btTransform(btQuaternion(0,0,0), btVector3(0.5,0.0,-0.35)), comp, 1.0, 1, true);
    //cam->setDisplayOnScreen(true);
    //AddSensor(cam);
    
    //DepthCamera* cam = new DepthCamera("Camera", 600, 400, 90.0, 0.1, 2.0, btTransform(btQuaternion(0,0,0), btVector3(0.5,0.0,-0.35)), comp, 1.0);
    //cam->setDisplayOnScreen(true);
    //AddSensor(cam);
    
	//Triggers
	//Trigger* trig = new Trigger("BoxTrigger", btVector3(1.0,1.0,1.0), btTransform(btQuaternion::getIdentity(), btVector3(0,0,5.0)));
	//trig->AddActiveSolid(comp);
	//trig->setRenderable(false);
	//AddEntity(trig);	
}

void UnderwaterTestManager::SimulationStepCompleted()
{
    UnderwaterVehicle* vehicle = (UnderwaterVehicle*)getEntity("AUV");
    std::cout << vehicle->getName();
}