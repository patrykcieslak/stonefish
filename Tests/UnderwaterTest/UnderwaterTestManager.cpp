//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Compound.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLSpotLight.h>
#include <graphics/OpenGLTrackball.h>
#include <utils/SystemUtil.hpp>
#include <entities/forcefields/Atmosphere.h>
#include <entities/forcefields/Ocean.h>
#include <entities/statics/Obstacle.h>
#include <actuators/Thruster.h>
#include <sensors/scalar/Pressure.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/scalar/DVL.h>
#include <sensors/scalar/FOG.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/GPS.h>
#include <sensors/Contact.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <actuators/Light.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <sensors/scalar/Accelerometer.h>
#include <entities/FeatherstoneEntity.h>
#include <entities/forcefields/Trigger.h>
#include <entities/forcefields/Pipe.h>
#include <entities/forcefields/Jet.h>
#include <sensors/scalar/Profiler.h>
#include <sensors/scalar/Multibeam.h>
#include <utils/UnitSystem.h>

UnderwaterTestManager::UnderwaterTestManager(sf::Scalar stepsPerSecond)
: SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE, sf::HydrodynamicsType::GEOMETRY_BASED)
{
}

void UnderwaterTestManager::BuildScenario()
{
    //General
    //getTrackball()->setEnabled(false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Dummy", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.8), 0.5);
    getMaterialManager()->CreateMaterial("Fiberglass", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.5), 0.3);
    getMaterialManager()->CreateMaterial("Rock", sf::UnitSystem::Density(sf::CGS, sf::MKS, 3.0), 0.8);
    
    getMaterialManager()->SetMaterialsInteraction("Dummy", "Dummy", 0.5, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
    
    getMaterialManager()->SetMaterialsInteraction("Fiberglass", "Dummy", 0.5, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Rock", "Dummy", 0.6, 0.4);
    getMaterialManager()->SetMaterialsInteraction("Rock", "Fiberglass", 0.6, 0.4);
    
    ///////LOOKS///////////
    int red = CreateLook(sf::Color::RGB(1.f, 0.f, 0.f), 0.5f, 0.f);
    int green = CreateLook(sf::Color::RGB(0.f, 1.f, 0.f), 0.5f, 0.f);
    int blue = CreateLook(sf::Color::RGB(0.f, 0.f, 1.f), 0.5f, 0.f);
    int yellow = CreateLook(sf::Color::RGB(1.f, 0.9f, 0.f), 0.3f, 0.f);
    int grey = CreateLook(sf::Color::RGB(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    int seabed = CreateLook(sf::Color::RGB(0.7f, 0.7f, 0.5f), 0.9f, 0.f);
    int propLook = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.3f, 0.f, 0.f, sf::GetDataPath() + "propeller_tex.png");
    int ductLook = CreateLook(sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.4f, 0.5f);
    int manipLook = CreateLook(sf::Color::RGB(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    int link4Look = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.6f, 0.8f, 0.f, sf::GetDataPath() + "link4_tex.png");
    int eeLook = CreateLook(sf::Color::RGB(0.59f, 0.56f, 0.51f), 0.6f, 0.8f);
    
    ////////OBJECTS    
    //Create environment
	EnableOcean(true);
    getOcean()->setWaterType(sf::Scalar(0.2));
    getAtmosphere()->getOpenGLAtmosphere()->SetSunPosition(0.0, 80.0);
    
    sf::Obstacle* terrain = new sf::Obstacle("Terrain", sf::GetDataPath() + "canyon.obj", 1.0, sf::I4(), getMaterialManager()->getMaterial("Rock"), seabed, false);
    AddStaticEntity(terrain, sf::Transform(sf::IQ(), sf::Vector3(0,0,4.0)));
    
    //sf::Plane* plane = new sf::Plane("Bottom", 1000.0, getMaterialManager()->getMaterial("Rock"), seabed);
    //AddStaticEntity(plane, sf::Transform(sf::Quaternion(0.0,0.0,M_PI/10.0), sf::Vector3(0,0,100.0)));
	
    sf::Obstacle* column = new sf::Obstacle("Col", sf::Scalar(0.2), sf::Scalar(5.0), getMaterialManager()->getMaterial("Rock"), grey);
    AddStaticEntity(column, sf::Transform(sf::IQ(), sf::Vector3(0,-2.0,0.5)));
    
    sf::Obstacle* sphere = new sf::Obstacle("Sph", sf::Scalar(0.5), getMaterialManager()->getMaterial("Rock"), red);
    AddStaticEntity(sphere, sf::Transform(sf::IQ(), sf::Vector3(-1.0,-2.0,1.0)));

    sphere = new sf::Obstacle("Sph2", sf::Scalar(0.5), getMaterialManager()->getMaterial("Rock"), green);
    AddStaticEntity(sphere, sf::Transform(sf::IQ(), sf::Vector3(0.0,-2.0,1.0)));
    
    sphere = new sf::Obstacle("Sph3", sf::Scalar(0.5), getMaterialManager()->getMaterial("Rock"), blue);
    AddStaticEntity(sphere, sf::Transform(sf::IQ(), sf::Vector3(1.0,-2.0,1.0)));
    
    /*sf::Box* box = new sf::Box("TestBox", sf::Vector3(1,2,0.5), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)), getMaterialManager()->getMaterial("Dummy"), yellow);
    AddSolidEntity(box, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)));
    box->SetArbitraryPhysicalProperties(box->getMass(), box->getInertia(), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,3.0)));*/
    
    /*sf::Polyhedron* sph = new sf::Polyhedron("Poly", sf::GetDataPath() + "sphere_R=1.obj", sf::Scalar(1),
                                             sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,1.0)), getMaterialManager()->getMaterial("Dummy"), grey);
    AddSolidEntity(sph, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)));*/
    
    /*sf::Polyhedron* sph = new sf::Polyhedron("Poly", sf::GetDataPath() + "sphere_R=1.obj", sf::Scalar(0.5), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)),
                                                     sf::GetDataPath() + "sphere_R=1.obj", sf::Scalar(0.5), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)),
                                                     getMaterialManager()->getMaterial("Dummy"), grey);
    AddSolidEntity(sph, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)));*/
    
    //sph->SetArbitraryPhysicalProperties(sph->getMass(), sph->getInertia(), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,1.0)));
    
    /*sf::Sphere* sph = new sf::Sphere("Ballast", sf::Scalar(0.3), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Rock"), grey);
    sf::Sphere* sph2 = new sf::Sphere("Ballast", sf::Scalar(0.3), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), grey);
    
    sf::Compound* comp = new sf::Compound("Boat", sph, sf::Transform(sf::IQ(), sf::Vector3(0,0,1.0)));
    comp->AddExternalPart(sph2, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,-1.0)));
    AddSolidEntity(comp, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)));*/
    
    //Pipe* vf = new Pipe(Vector3(10.0, 0.0, 1.0), Vector3(-10.0, 0.0, 1.0), 1.0, 1.0, 1.0, 1.0);
    //getOcean()->AddVelocityField(vf);
    
    //Jet* vf2 = new Jet(Vector3(0.0, -3.0, 1.0),Vector3(0,1.0,0), 0.1, 10.0);
    //getOcean()->AddVelocityField(vf2);
    
	//Obstacle* bedrock = new Obstacle("Bedrock", GetDataPath() + "canyon.obj", 1.0, getMaterialManager()->getMaterial("Rock"), grey, false);
	//AddStaticEntity(bedrock, Transform(Quaternion(0.0,0.0,-M_PI_2), Vector3(0,0,7.0)));
    
    //std::vector<StaticEntity*> group;
    
    /*for(unsigned int i=0; i<10; ++i)
    {
        Obstacle* cyl = new Obstacle("Rock", 1.0,3.0, getMaterialManager()->getMaterial("Rock"), seabed);
        AddStaticEntity(cyl, Transform(Quaternion::getIdentity(), Vector3(i*2.0,0,5.5)));    
        //group.push_back(cyl);
    }*/
    /*
    StaticEntity::GroupTransform(group, Transform(Quaternion::getIdentity(), Vector3(9.0,0.0,0.0)), Transform(Quaternion(M_PI_2,0,0), Vector3(-4.0,0.0,0.0)));
        
    Box* box = new Box("Test", Vector3(1.0,1.0,0.5), getMaterialManager()->getMaterial("Rock"), propLook);
    AddSolidEntity(box, Transform(Quaternion::getIdentity(), Vector3(0,0,3.0)));*/

	//Create underwater vehicle body
    //Externals
    /*sf::Polyhedron* hullB = new sf::Polyhedron("HullBottom", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, sf::Scalar(0.003), false);
    sf::Polyhedron* hullP = new sf::Polyhedron("HullPort", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, sf::Scalar(0.003), false);
    sf::Polyhedron* hullS = new sf::Polyhedron("HullStarboard", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, sf::Scalar(0.003), false);
    sf::Polyhedron* vBarStern = new sf::Polyhedron("VBarStern", sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), grey, false);
    sf::Polyhedron* vBarBow = new sf::Polyhedron("VBarBow", sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), grey, false);
    sf::Polyhedron* ductSway = new sf::Polyhedron("DuctSway", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductSurgeP = new sf::Polyhedron("DuctSurgePort", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductSurgeS = new sf::Polyhedron("DuctSurgeStarboard", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductHeaveS = new sf::Polyhedron("DuctHeaveStern", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductHeaveB = new sf::Polyhedron("DuctHeaveBow", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    //Internals
    sf::Cylinder* batteryCyl = new sf::Cylinder("BatteryCylinder", 0.13, 0.6, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(47));
    sf::Cylinder* portCyl = new sf::Cylinder("PortCylinder", 0.13, 1.0, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    portCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(35));
    sf::Cylinder* starboardCyl = new sf::Cylinder("StarboardCylinder", 0.13, 1.0, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    starboardCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(35));
    
    //Build whole body
    sf::Compound* comp = new sf::Compound("Compound", hullB, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,0)));
    comp->AddExternalPart(hullP, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,-0.35,-0.7)));
    comp->AddExternalPart(hullS, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0.35,-0.7)));
    comp->AddExternalPart(vBarStern, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(-0.25,0.0,-0.15)));
    comp->AddExternalPart(vBarBow, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0.30,0.0,-0.15)));
    comp->AddExternalPart(ductSway, sf::Transform(sf::Quaternion(M_PI_2,M_PI,0), sf::Vector3(-0.0137, 0.0307, -0.38)));
    comp->AddExternalPart(ductSurgeP, sf::Transform(sf::Quaternion(0,0,M_PI), sf::Vector3(-0.2807,-0.2587,-0.38)));
    comp->AddExternalPart(ductSurgeS, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(-0.2807,0.2587,-0.38)));
    comp->AddExternalPart(ductHeaveS, sf::Transform(sf::Quaternion(M_PI_2,-M_PI_2,0), sf::Vector3(-0.5337,0.0,-0.6747)));
    comp->AddExternalPart(ductHeaveB, sf::Transform(sf::Quaternion(-M_PI_2,-M_PI_2,0), sf::Vector3(0.5837,0.0,-0.6747)));
    comp->AddInternalPart(batteryCyl, sf::Transform(sf::Quaternion(M_PI_2,0,0), sf::Vector3(-0.1,0,0)));
    comp->AddInternalPart(portCyl, sf::Transform(sf::Quaternion(M_PI_2,0,0), sf::Vector3(0.0,-0.35,-0.7)));
    comp->AddInternalPart(starboardCyl, sf::Transform(sf::Quaternion(M_PI_2,0,0), sf::Vector3(0.0,0.35,-0.7)));
    
    //Manipulator bodies
    sf::Polyhedron* baseLink = new sf::Polyhedron("ArmBaseLink", sf::GetDataPath() + "base_link_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook, false);
    sf::Polyhedron* link1 = new sf::Polyhedron("ArmLink1", sf::GetDataPath() + "link1_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true, sf::HYDRO_PROXY_CYLINDER);
    sf::Polyhedron* link2 = new sf::Polyhedron("ArmLink2", sf::GetDataPath() + "link2_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true, sf::HYDRO_PROXY_CYLINDER);
    sf::Polyhedron* link3 = new sf::Polyhedron("ArmLink3", sf::GetDataPath() + "link3_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true);//, HYDRO_PROXY_CYLINDER);
    sf::Polyhedron* link4 = new sf::Polyhedron("ArmLink4", sf::GetDataPath() + "link4ft_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), link4Look, false, -1, true);//, HYDRO_PROXY_CYLINDER);
    
    //Create thrusters
    sf::Polyhedron* prop1 = new sf::Polyhedron("Propeller", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop2 = new sf::Polyhedron("Propeller", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop3 = new sf::Polyhedron("Propeller", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop4 = new sf::Polyhedron("Propeller", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop5 = new sf::Polyhedron("Propeller", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Thruster* thSway = new sf::Thruster("ThrusterSway", prop1, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thSurgeP = new sf::Thruster("ThrusterSurgePort", prop2, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thSurgeS = new sf::Thruster("ThrusterSurgeStarboard", prop3, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thHeaveS = new sf::Thruster("ThrusterHeaveStern", prop4, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thHeaveB = new sf::Thruster("ThrusterHeaveBow", prop5, 0.18, 0.48, 0.05, 1000.0);
    
    //Create gripper body
    sf::Polyhedron* eeBase = new sf::Polyhedron("EEBase", sf::GetDataPath() + "eeprobe_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), eeLook, false);
    sf::Sphere* eeTip = new sf::Sphere("EETip", 0.015, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), eeLook);
    sf::Compound* ee = new sf::Compound("EE", eeBase, sf::Transform::getIdentity());
    ee->AddExternalPart(eeTip, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0,0,0.124)));
     */
    //Create underwater vehicle
    /*
#ifdef USE_IAUV_CLASSES
    Scalar depth = 0.7;
    
	UnderwaterVehicle* vehicle = new UnderwaterVehicle("AUV", comp);
	AddSystemEntity(vehicle, Transform(Quaternion(0,0,0), Vector3(0,0,depth)));
    Accelerometer* acc = new Accelerometer("Acc", comp, Transform::getIdentity(), -1, 0);
    AddSensor(acc);
    
    //Add sensors
	Odometry* odom = vehicle->AddOdometry(Transform::getIdentity());
    Pressure* press = vehicle->AddPressureSensor(Transform(Quaternion::getIdentity(), Vector3(0,0,0)), 1.0);
    press->SetNoise(1.0);
    DVL* dvl = vehicle->AddDVL(Transform(Quaternion(0,0,M_PI), Vector3(0,0,0)), UnitSystem::Angle(true, 30.0));
    dvl->SetNoise(0.02, 0.05);
    IMU* imu = vehicle->AddIMU(Transform(Quaternion::getIdentity(), Vector3(0,0,0)));
    imu->SetNoise(0.01, 0.05);
    FOG* fog = vehicle->AddFOG(Transform(Quaternion::getIdentity(), Vector3(0,0,0)));
    fog->SetNoise(0.001);
    GPS* gps = vehicle->AddGPS(Transform(Quaternion::getIdentity(), Vector3(0,0,-1)), 41.77737, 3.03376);
    gps->SetNoise(0.5);

    //Attach thrusters
    vehicle->AddThruster(thSway, Transform(Quaternion(M_PI_2,M_PI,0), Vector3(-0.0137, 0.0307, -0.38)));
    vehicle->AddThruster(thSurgeP, Transform(Quaternion(0,0,0), Vector3(-0.2807,-0.2587,-0.38)));
    vehicle->AddThruster(thSurgeS, Transform(Quaternion(0,0,0), Vector3(-0.2807,0.2587,-0.38)));
    vehicle->AddThruster(thHeaveS, Transform(Quaternion(0,-M_PI_2,0), Vector3(-0.5337,0.0,-0.6747)));
    vehicle->AddThruster(thHeaveB, Transform(Quaternion(0,-M_PI_2,0), Vector3(0.5837,0.0,-0.6747)));
    
    FakeRotaryEncoder* enc = new FakeRotaryEncoder("Encoder", thSway, -1, 400);
    AddSensor(enc);
    
    //Create manipulator
    Manipulator* arm = new Manipulator("Arm", 4, baseLink, Transform(Quaternion::getIdentity(), Vector3(0,0,0)), vehicle->getVehicleBody());
    arm->AddRotLinkDH("Link1", link1, Transform(Quaternion(0,0,0), Vector3(0,0,0)), -0.0136, 0.1065, M_PI_2);
	arm->AddRotLinkDH("Link2", link2, Transform(Quaternion(0,0,M_PI_2), Vector3(0,0,0)), 0, 0.23332, 0);
	arm->AddRotLinkDH("Link3", link3, Transform(Quaternion(0,0,M_PI_2), Vector3(0,0,0)), 0, 0.103, -M_PI_2);
    arm->AddTransformDH(0.201,0,0);
    arm->AddRotLinkDH("Link4", link4, Transform(Quaternion(0,0,0), Vector3(0,0,0)), 0, 0, 0);
	AddSystemEntity(arm, Transform(Quaternion(0,0,0), Vector3(0.90,0.0,depth)));
    
    //Add end-effector with force sensor
    //FixedGripper* gripper = new FixedGripper("Gripper", arm, ee);
    Box* eeBase0 = new Box("EEBase", Vector3(0.02,0.02,0.02), Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    Box* eeFinger1_ = new Box("EEFinger1_", Vector3(0.02,0.1,0.2), Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    Box* eeFinger2_ = new Box("EEFinger2_", Vector3(0.02,0.1,0.2), Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    Compound* eeFinger1 = new Compound("EEFinger1", eeFinger1_, Transform(Quaternion::getIdentity(), Vector3(0.04, 0.0, 0.1)));
    Compound* eeFinger2 = new Compound("EEFinger2", eeFinger2_, Transform(Quaternion::getIdentity(), Vector3(-0.04, 0.0, 0.1)));
    
    TwoFingerGripper* gripper = new TwoFingerGripper("Gripper", arm, eeBase0, eeFinger1, eeFinger2, Vector3(0.04, 0.0, 0.0), Vector3(-0.04, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 0.3, 10.0);
    AddSystemEntity(gripper, Transform(Quaternion::getIdentity(), Vector3(0,0, 0.4)));
    
    //Add contact sensing between gripper and target
    Contact* cnt = AddContact(ee, plane, 10000);
    cnt->setDisplayMask(CONTACT_DISPLAY_PATH_B);
#else
    comp->AddExternalPart(baseLink, Transform(Quaternion::getIdentity(), Vector3(0.74, 0.0, 0.0)));
    
    //Build robot rigid body tree
    FeatherstoneEntity* iauv = new FeatherstoneEntity("IAUV", 5, comp, getDynamicsWorld(), false);
    iauv->setSelfCollision(true);
    iauv->AddLink(link1, Transform(Quaternion::getIdentity(), Vector3(0.74, 0.0, 0.0)), getDynamicsWorld());
    iauv->AddLink(link2, Transform(Quaternion::getIdentity(), Vector3(0.74 + 0.1065, 0.0, 0.0)), getDynamicsWorld());
    iauv->AddLink(link3, Transform(Quaternion::getIdentity(), Vector3(0.74 + 0.1065 + 0.23332, 0.0, 0.0)), getDynamicsWorld());
    iauv->AddLink(link4, Transform(Quaternion::getIdentity(), Vector3(0.74 + 0.1065 + 0.23332 + 0.103, 0.0, 0.201)), getDynamicsWorld());
    iauv->AddRevoluteJoint(0, 1, Vector3(0.74, 0.0, 0.0), Vector3(0.0,0.0,1.0));
    iauv->AddRevoluteJoint(1, 2, Vector3(0.74 + 0.1065, 0.0, 0.0), Vector3(0.0,1.0,0.0));
    iauv->AddRevoluteJoint(2, 3, Vector3(0.74 + 0.1065 + 0.23332, 0.0, 0.0), Vector3(0.0,1.0,0.0));
    iauv->AddRevoluteJoint(3, 4, Vector3(0.74 + 0.1065 + 0.23332 + 0.103, 0.0, 0.201), Vector3(0.0,0.0,1.0));
    AddFeatherstoneEntity(iauv, Transform(Quaternion(0,0,0), Vector3(0,0,0)));
    
    //Add end-effector with force sensor
    FeatherstoneEntity* effector = new FeatherstoneEntity("EndEffector", 1, ee, getDynamicsWorld(), false);
    AddFeatherstoneEntity(effector, Transform(Quaternion::getIdentity(), Vector3(0.74 + 0.1065 + 0.23332 + 0.103, 0.0, 0.201+0.05)));
    FixedJoint* ftFix = new FixedJoint("Fix", effector, iauv, -1, 3);
    AddJoint(ftFix);
    ForceTorque* ft = new ForceTorque("FT", ftFix, effector->getLink(0).solid, Transform::getIdentity());
    AddSensor(ft);
    
    //Add sensors
    Pressure* press = new Pressure("Pressure", comp, Transform::getIdentity());
    AddSensor(press);
    DVL* dvl = new DVL("DVL", comp, Transform(Quaternion(0,0,M_PI), Vector3(0,0,0)), UnitSystem::Angle(true, 30.0));
    AddSensor(dvl);
    IMU* imu = new IMU("IMU", comp, Transform::getIdentity());
    AddSensor(imu);
    FOG* fog = new FOG("FOG", comp, Transform::getIdentity());
    AddSensor(fog);
    GPS* gps = new GPS("GPS", 20.0, 0.0, comp, Transform::getIdentity());
    AddSensor(gps);
    
    //Attach thrusters
    thSway->AttachToSolid(iauv, 0, Transform(Quaternion(M_PI_2,M_PI,0), Vector3(-0.0137, 0.0307, -0.38)));
    thSurgeP->AttachToSolid(iauv, 0, Transform(Quaternion(0,0,0), Vector3(-0.2807,-0.2587,-0.38)));
    thSurgeS->AttachToSolid(iauv, 0, Transform(Quaternion(0,0,0), Vector3(-0.2807,0.2587,-0.38)));
    thHeaveS->AttachToSolid(iauv, 0, Transform(Quaternion(0,-M_PI_2,0), Vector3(-0.5337,0.0,-0.6747)));
    thHeaveB->AttachToSolid(iauv, 0, Transform(Quaternion(0,-M_PI_2,0), Vector3(0.5837,0.0,-0.6747)));
    AddActuator(thSway);
    AddActuator(thSurgeP);
    AddActuator(thSurgeS);
    AddActuator(thHeaveS);
    AddActuator(thHeaveB);
    
#endif
    */
    //Profiler* prof = new Profiler("Laser", comp, Transform(Quaternion(0,0,0), Vector3(0,0,0.5)), 50.0, 100, 100.0);
    //AddSensor(prof);
    //Multibeam* mb = new Multibeam("Multibeam", comp, Transform(Quaternion(0,0,0), Vector3(0,0,0.5)), 120.0, 400, 10.0);
    //mb->SetRange(0.2, 10.0);
    //AddSensor(mb);
    
    //ColorCamera* cam = new ColorCamera("Camera", 600, 400, 90.0, Transform(Quaternion(0,0,0), Vector3(0.5,0.0,-0.35)), comp, 1.0, 1, true);
    //cam->setDisplayOnScreen(true);
    //AddSensor(cam);
    
    //DepthCamera* cam = new DepthCamera("Camera", 600, 400, 90.0, 0.1, 2.0, Transform(Quaternion(0,0,0), Vector3(0.5,0.0,-0.35)), comp, 1.0);
    //cam->setDisplayOnScreen(true);
    //AddSensor(cam);
    
	//Triggers
	//Trigger* trig = new Trigger("BoxTrigger", Vector3(1.0,1.0,1.0), Transform(Quaternion::getIdentity(), Vector3(0,0,5.0)));
	//trig->AddActiveSolid(comp);
	//trig->setRenderable(false);
     //AddEntity(trig);
}

void UnderwaterTestManager::SimulationStepCompleted()
{
}
