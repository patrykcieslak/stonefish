//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include <core/Robot.h>
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Compound.h>
#include <entities/solids/Wing.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLSpotLight.h>
#include <graphics/OpenGLTrackball.h>
#include <utils/SystemUtil.hpp>
#include <entities/statics/Obstacle.h>
#include <entities/statics/Terrain.h>
#include <actuators/Thruster.h>
#include <actuators/ServoMotor.h>
#include <sensors/scalar/Pressure.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/scalar/DVL.h>
#include <sensors/scalar/FOG.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/GPS.h>
#include <sensors/Contact.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/Multibeam2.h>
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
    getTrackball()->setEnabled(true);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Dummy", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.9), 0.5);
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
    int grid = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.5f, 0.f, 0.f, sf::GetShaderPath() + "grid.png");
    
    ////////OBJECTS    
    //Create environment
	EnableOcean(false);
    getOcean()->SetupWaterProperties(0.2, 1.0);
    getAtmosphere()->SetupSunPosition(0.0, 60.0);
    
    //sf::Plane* plane = new sf::Plane("Bottom", 10000.0, getMaterialManager()->getMaterial("Rock"), seabed);
    //AddStaticEntity(plane, sf::Transform(sf::IQ(), sf::Vector3(0,0,5)));
    
    sf::Terrain* terrain = new sf::Terrain("Terrain", sf::GetDataPath() + "terrain_small.png",
                                           sf::Scalar(1), sf::Scalar(1), sf::Scalar(5), getMaterialManager()->getMaterial("Rock"), seabed);
    AddStaticEntity(terrain, sf::Transform(sf::IQ(), sf::Vector3(0,0,5)));
    
    //sf::Obstacle* box = new sf::Obstacle("Box", sf::Vector3(0.2,0.2,0.2), getMaterialManager()->getMaterial("Rock"), seabed);
    //AddStaticEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0,0,5)));
    
	//Create underwater vehicle body
    //Externals
    sf::Polyhedron* hullB = new sf::Polyhedron("HullBottom", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, sf::Scalar(0.003), true, false);
    sf::Polyhedron* hullP = new sf::Polyhedron("HullPort", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, sf::Scalar(0.003), true, false);
    sf::Polyhedron* hullS = new sf::Polyhedron("HullStarboard", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), yellow, false, sf::Scalar(0.003), true, false);
    sf::Polyhedron* vBarStern = new sf::Polyhedron("VBarStern", sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), grey, false, sf::Scalar(0.003), true, false);
    sf::Polyhedron* vBarBow = new sf::Polyhedron("VBarBow", sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), grey, false, sf::Scalar(0.003), true, false);
    sf::Polyhedron* ductSway = new sf::Polyhedron("DuctSway", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductSurgeP = new sf::Polyhedron("DuctSurgePort", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductSurgeS = new sf::Polyhedron("DuctSurgeStarboard", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductHeaveS = new sf::Polyhedron("DuctHeaveStern", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    sf::Polyhedron* ductHeaveB = new sf::Polyhedron("DuctHeaveBow", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), ductLook, false);
    //Internals
    sf::Cylinder* batteryCyl = new sf::Cylinder("BatteryCylinder", 0.13, 0.6, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(70));
    sf::Cylinder* portCyl = new sf::Cylinder("PortCylinder", 0.13, 1.0, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    portCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(20));
    sf::Cylinder* starboardCyl = new sf::Cylinder("StarboardCylinder", 0.13, 1.0, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    starboardCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(20));
    
    //Build whole body
    sf::Compound* vehicle = new sf::Compound("Vehicle", hullB, sf::I4());
    vehicle->AddExternalPart(hullP, sf::Transform(sf::IQ(), sf::Vector3(0,-0.35,-0.7)));
    vehicle->AddExternalPart(hullS, sf::Transform(sf::IQ(), sf::Vector3(0,0.35,-0.7)));
    vehicle->AddExternalPart(vBarStern, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(-0.25,0.0,-0.15)));
    vehicle->AddExternalPart(vBarBow, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0.30,0.0,-0.15)));
    vehicle->AddExternalPart(ductSway, sf::Transform(sf::Quaternion(M_PI_2,M_PI,0), sf::Vector3(-0.0137, 0.0307, -0.38)));
    vehicle->AddExternalPart(ductSurgeP, sf::Transform(sf::Quaternion(0,0,M_PI), sf::Vector3(-0.2807,-0.2587,-0.38)));
    vehicle->AddExternalPart(ductSurgeS, sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(-0.2807,0.2587,-0.38)));
    vehicle->AddExternalPart(ductHeaveS, sf::Transform(sf::Quaternion(M_PI_2,-M_PI_2,0), sf::Vector3(-0.5337,0.0,-0.6747)));
    vehicle->AddExternalPart(ductHeaveB, sf::Transform(sf::Quaternion(-M_PI_2,-M_PI_2,0), sf::Vector3(0.5837,0.0,-0.6747)));
    vehicle->AddInternalPart(batteryCyl, sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(-0.1,0,0)));
    vehicle->AddInternalPart(portCyl, sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,-0.35,-0.7)));
    vehicle->AddInternalPart(starboardCyl, sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,0.35,-0.7)));
    
    //Manipulator bodies
    sf::Polyhedron* baseLink = new sf::Polyhedron("ArmBaseLink", sf::GetDataPath() + "base_link_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook, false, -1, true, true, sf::HYDRO_PROXY_SPHERE);
    sf::Polyhedron* link1 = new sf::Polyhedron("ArmLink1", sf::GetDataPath() + "link1_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    sf::Polyhedron* link2 = new sf::Polyhedron("ArmLink2", sf::GetDataPath() + "link2_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    sf::Polyhedron* link3 = new sf::Polyhedron("ArmLink3", sf::GetDataPath() + "link3_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), manipLook);
    sf::Polyhedron* link4 = new sf::Polyhedron("ArmLink4", sf::GetDataPath() + "link4ft_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), link4Look);
    
    //Create a fin
    //sf::Wing* wing = new sf::Wing("Wing1", 0.3, 0.2, "0010", 0.3, sf::I4(), getMaterialManager()->getMaterial("Dummy"), red);
    
    std::vector<sf::SolidEntity*> arm;
    arm.push_back(baseLink);
    arm.push_back(link1);
    arm.push_back(link2);
    arm.push_back(link3);
    arm.push_back(link4);
    //arm.push_back(wing);
    
    //Create manipulator servomotors
    sf::ServoMotor* srv1 = new sf::ServoMotor("Servo1", 1.0, 1.0, 10.0);
    sf::ServoMotor* srv2 = new sf::ServoMotor("Servo2", 1.0, 1.0, 10.0);
    sf::ServoMotor* srv3 = new sf::ServoMotor("Servo3", 1.0, 1.0, 10.0);
    sf::ServoMotor* srv4 = new sf::ServoMotor("Servo4", 1.0, 1.0, 10.0);
    
    //Create thrusters
    sf::Polyhedron* prop1 = new sf::Polyhedron("Propeller1", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop2 = new sf::Polyhedron("Propeller2", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop3 = new sf::Polyhedron("Propeller3", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop4 = new sf::Polyhedron("Propeller4", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Polyhedron* prop5 = new sf::Polyhedron("Propeller5", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), propLook, false);
    sf::Thruster* thSway = new sf::Thruster("ThrusterSway", prop1, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thSurgeP = new sf::Thruster("ThrusterSurgePort", prop2, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thSurgeS = new sf::Thruster("ThrusterSurgeStarboard", prop3, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thHeaveS = new sf::Thruster("ThrusterHeaveStern", prop4, 0.18, 0.48, 0.05, 1000.0);
    sf::Thruster* thHeaveB = new sf::Thruster("ThrusterHeaveBow", prop5, 0.18, 0.48, 0.05, 1000.0);
    
    //Create ligths
    //sf::Light* spot1 = new sf::Light("Spot1", sf::Color::BlackBody(4000.0), 100000.0); //OMNI
    //sf::Light* spot1 = new sf::Light("Spot1", 30.0, sf::Color::BlackBody(4000.0), 100000.0);
    
    //Create sensors
    sf::Odometry* odom = new sf::Odometry("Odom");
    sf::Pressure* press = new sf::Pressure("Pressure");
    press->SetNoise(1.0);
    sf::DVL* dvl = new sf::DVL("DVL", 30.0);
    dvl->SetNoise(0.02, 0.05);
    sf::IMU* imu = new sf::IMU("IMU");
    imu->SetNoise(0.01, 0.05);
    sf::FOG* fog = new sf::FOG("FOG");
    fog->SetNoise(0.01);
    sf::GPS* gps = new sf::GPS("GPS", 41.77737, 3.03376);
    gps->SetNoise(0.5);
    //sf::ColorCamera* cam = new sf::ColorCamera("Camera", 300, 200, 50.0);
    //cam->setDisplayOnScreen(true);
    //sf::Multibeam2* mb = new sf::Multibeam2("MB", 400, 200, 170.0, 30.0, 0.5, 5.0);
    //mb->setDisplayOnScreen(true);
    //sf::DepthCamera* dcam = new sf::DepthCamera("Camera", 200, 200, 90.0, 0.5, 5.0);
    //dcam->setDisplayOnScreen(false);
    //sf::DepthCamera* dcam2 = new sf::DepthCamera("Camera", 200, 200, 90.0, 0.5, 5.0);
    //dcam2->setDisplayOnScreen(true);
    
    //Create gripper body
    /*sf::Polyhedron* eeBase = new sf::Polyhedron("EEBase", sf::GetDataPath() + "eeprobe_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), eeLook, false);
    sf::Sphere* eeTip = new sf::Sphere("EETip", 0.015, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), eeLook);
    sf::Compound* ee = new sf::Compound("EE", eeBase, sf::Transform::getIdentity());
    ee->AddExternalPart(eeTip, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0,0,0.124)));
    */
    
    //Create AUV
    sf::Robot* auv = new sf::Robot("GIRONA500");
    
    //Mechanical structure
    auv->DefineLinks(vehicle, arm);
    auv->DefineFixedJoint("VehicleToArm", "Vehicle", "ArmBaseLink", sf::Transform(sf::IQ(), sf::Vector3(0.74,0.0,0.0)));
    auv->DefineRevoluteJoint("Joint1", "ArmBaseLink", "ArmLink1", sf::I4(), sf::Vector3(0.0, 0.0, 1.0), std::make_pair(1.0, -1.0));
    auv->DefineRevoluteJoint("Joint2", "ArmLink1", "ArmLink2", sf::Transform(sf::IQ(), sf::Vector3(0.1065, 0.0, 0.0)), sf::Vector3(0.0, 1.0, 0.0), std::make_pair(1.0, -1.0));
    auv->DefineRevoluteJoint("Joint3", "ArmLink2", "ArmLink3", sf::Transform(sf::IQ(), sf::Vector3(0.23332, 0.0, 0.0)), sf::Vector3(0.0, 1.0, 0.0), std::make_pair(1.0, -1.0));
    auv->DefineRevoluteJoint("Joint4", "ArmLink3", "ArmLink4", sf::Transform(sf::IQ(), sf::Vector3(0.103, 0.0, 0.201)), sf::Vector3(0.0, 0.0, 1.0), std::make_pair(1.0, -1.0));
    //auv->DefineRevoluteJoint("WingJoint", "Vehicle", "Wing1", sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,1.0)), sf::Vector3(0.0,1.0,0.0), std::make_pair(1.0, -1.0));
    
    //Joint motors
    auv->AddJointActuator(srv1, "Joint1");
    auv->AddJointActuator(srv2, "Joint2");
    auv->AddJointActuator(srv3, "Joint3");
    auv->AddJointActuator(srv4, "Joint4");
    
    //Thrusters
    auv->AddLinkActuator(thSway, "Vehicle", sf::Transform(sf::Quaternion(M_PI_2,M_PI,0), sf::Vector3(-0.0137, 0.0307, -0.38)));
    auv->AddLinkActuator(thSurgeP, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.2807,-0.2587,-0.38)));
    auv->AddLinkActuator(thSurgeS, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.2807,0.2587,-0.38)));
    auv->AddLinkActuator(thHeaveS, "Vehicle", sf::Transform(sf::Quaternion(0,-M_PI_2,0), sf::Vector3(-0.5337,0.0,-0.6747)));
    auv->AddLinkActuator(thHeaveB, "Vehicle", sf::Transform(sf::Quaternion(0,-M_PI_2,0), sf::Vector3(0.5837,0.0,-0.6747)));
  
    //Lights
    //auv->AddLinkActuator(spot1, "Vehicle", sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0,0,2.5)));
    
    //Sensors
    auv->AddLinkSensor(odom, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0,0,0)));
    auv->AddLinkSensor(press, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0.6,0,-0.7)));
    auv->AddLinkSensor(dvl, "Vehicle", sf::Transform(sf::Quaternion(0,0,M_PI), sf::Vector3(-0.5,0,0.1)));
    auv->AddLinkSensor(imu, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0,0,-0.7)));
    auv->AddLinkSensor(fog, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0.3,0,-0.7)));
    auv->AddLinkSensor(gps, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.5,0,-0.9)));
    //auv->AddVisionSensor(cam, "Vehicle", sf::Transform(sf::Quaternion(M_PI_2,0,M_PI_2), sf::Vector3(0.5,0.0,-0.35)));
    //auv->AddVisionSensor(mb, "Vehicle", sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0.7,0.0,0.3)));
    //auv->AddVisionSensor(dcam, "Vehicle", sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0.7,0.0,0.3)));
    //auv->AddVisionSensor(dcam2, "Vehicle", sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0.7,0.0,0.3)));

    
    AddRobot(auv, sf::Transform(sf::Quaternion(0,0,0.5), sf::Vector3(5,0,1)));
    
    /*sf::Box* baseLink2 = new sf::Box("BaseLink", sf::Vector3(0.01,0.01,0.01), sf::I4(), getMaterialManager()->getMaterial("Rock"), red);
    sf::Box* link12 = new sf::Box("Link1", sf::Vector3(0.01,0.01,0.5), sf::I4(), getMaterialManager()->getMaterial("Rock"), red);
    
    sf::FeatherstoneEntity* fe = new sf::FeatherstoneEntity("Manipulator1", 2, baseLink2, true);
    fe->AddLink(link12, sf::Transform(sf::Quaternion(0,0,M_PI_2), sf::Vector3(0,-0.25,0)));
    fe->AddRevoluteJoint("Joint1", 0, 1, sf::Vector3(0,0,0), sf::Vector3(1,0,0));
    AddFeatherstoneEntity(fe, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0,0,3.0)));*/
    
    //sf::Sphere* sph = new sf::Sphere("Sph", 0.5, sf::I4(), getMaterialManager()->getMaterial("Dummy"), red);
    //AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0,0,-2.0)));
    
    //Create manipulator
    /*Manipulator* arm = new Manipulator("Arm", 4, baseLink, Transform(Quaternion::getIdentity(), Vector3(0,0,0)), vehicle->getVehicleBody());
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
    cnt->setDisplayMask(CONTACT_DISPLAY_PATH_B);*/
    
    //Profiler* prof = new Profiler("Laser", comp, Transform(Quaternion(0,0,0), Vector3(0,0,0.5)), 50.0, 100, 100.0);
    //AddSensor(prof);
    //Multibeam* mb = new Multibeam("Multibeam", comp, Transform(Quaternion(0,0,0), Vector3(0,0,0.5)), 120.0, 400, 10.0);
    //mb->SetRange(0.2, 10.0);
    //AddSensor(mb);
    
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
