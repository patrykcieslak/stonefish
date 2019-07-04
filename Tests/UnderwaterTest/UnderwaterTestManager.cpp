/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  UnderwaterTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2019 Patryk Cieslak. All rights reserved.
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
: SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE, sf::FluidDynamicsType::GEOMETRY_BASED)
{
}

void UnderwaterTestManager::BuildScenario()
{
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
    int yellow = CreateLook(sf::Color::RGB(1.f, 0.9f, 0.f), 0.3f, 0.f);
    int grey = CreateLook(sf::Color::RGB(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    int seabed = CreateLook(sf::Color::RGB(0.7f, 0.7f, 0.5f), 0.9f, 0.f);
    int propLook = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.3f, 0.f, 0.f, sf::GetDataPath() + "propeller_tex.png");
    int ductLook = CreateLook(sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.4f, 0.5f);
    int manipLook = CreateLook(sf::Color::RGB(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    int link4Look = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.6f, 0.8f, 0.f, sf::GetDataPath() + "link4_tex.png");
    
    ////////OBJECTS    
    //Create environment
	EnableOcean(0.0);
    getOcean()->SetupWaterProperties(0.2, 1.0);
    getAtmosphere()->SetupSunPosition(0.0, 60.0);
    
    sf::Plane* plane = new sf::Plane("Bottom", 10000.0, getMaterialManager()->getMaterial("Rock"), seabed);
    AddStaticEntity(plane, sf::Transform(sf::IQ(), sf::Vector3(0,0,5)));
    
    /*sf::Box* box = new sf::Box("Box", sf::Vector3(0.5,0.5,0.5), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    box->ScalePhysicalPropertiesToArbitraryMass(100.0);
    AddSolidEntity(box, sf::Transform(sf::Quaternion(0.0,0.3,0.0), sf::Vector3(0,0,1.0)));*/
    
    /*sf::Cylinder* batteryCyl = new sf::Cylinder("BatteryCylinder", 0.13, 0.6, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(25));
    
    sf::Cylinder* batteryCyl2 = new sf::Cylinder("BatteryCylinder2", 0.13, 0.6, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    batteryCyl2->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(25));
    
    sf::Compound* vehicle = new sf::Compound("Vehicle", batteryCyl, sf::I4(), sf::BodyPhysicsType::SUBMERGED_BODY);
    vehicle->AddExternalPart(batteryCyl2, sf::Transform(sf::IQ(), sf::Vector3(0,0.5,0)));
    
    AddSolidEntity(vehicle, sf::Transform(sf::IQ(), sf::Vector3(0,0,1.0)));*/
    
    //sf::Terrain* terrain = new sf::Terrain("Terrain", sf::GetDataPath() + "terrain_small.png",
    //                                       sf::Scalar(1), sf::Scalar(1), sf::Scalar(5), getMaterialManager()->getMaterial("Rock"), seabed);
    //AddStaticEntity(terrain, sf::Transform(sf::IQ(), sf::Vector3(0,0,5)));
    
    //sf::Obstacle* box = new sf::Obstacle("Box", sf::Vector3(0.2,0.2,0.2), getMaterialManager()->getMaterial("Rock"), seabed);
    //AddStaticEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0,0,5)));
    
	//Create underwater vehicle body
    //Externals
    sf::Polyhedron* hullB = new sf::Polyhedron("HullBottom", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), sf::BodyPhysicsType::SUBMERGED_BODY, yellow, false, sf::Scalar(0.003), false);
    sf::Polyhedron* hullP = new sf::Polyhedron("HullPort", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), sf::BodyPhysicsType::SUBMERGED_BODY, yellow, false, sf::Scalar(0.003), false);
    sf::Polyhedron* hullS = new sf::Polyhedron("HullStarboard", sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), sf::BodyPhysicsType::SUBMERGED_BODY, yellow, false, sf::Scalar(0.003), false);
    sf::Polyhedron* vBarStern = new sf::Polyhedron("VBarStern", sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, grey, false, sf::Scalar(0.003), false);
    sf::Polyhedron* vBarBow = new sf::Polyhedron("VBarBow", sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, grey, false, sf::Scalar(0.003), false);
    sf::Polyhedron* ductSway = new sf::Polyhedron("DuctSway", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, ductLook);
    sf::Polyhedron* ductSurgeP = new sf::Polyhedron("DuctSurgePort", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, ductLook);
    sf::Polyhedron* ductSurgeS = new sf::Polyhedron("DuctSurgeStarboard", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, ductLook);
    sf::Polyhedron* ductHeaveS = new sf::Polyhedron("DuctHeaveStern", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, ductLook);
    sf::Polyhedron* ductHeaveB = new sf::Polyhedron("DuctHeaveBow", sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, ductLook);
    //Internals
    sf::Cylinder* batteryCyl = new sf::Cylinder("BatteryCylinder", 0.13, 0.6, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(70));
    sf::Cylinder* portCyl = new sf::Cylinder("PortCylinder", 0.13, 1.0, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    portCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(20));
    sf::Cylinder* starboardCyl = new sf::Cylinder("StarboardCylinder", 0.13, 1.0, sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    starboardCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(20));
    
    //Build whole body
    sf::Compound* vehicle = new sf::Compound("Vehicle", hullB, sf::I4(), sf::BodyPhysicsType::SUBMERGED_BODY);
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
    
    vehicle->setDisplayInternalParts(false);
    
    //Manipulator bodies
    sf::Polyhedron* baseLink = new sf::Polyhedron("ArmBaseLink", sf::GetDataPath() + "base_link_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    sf::Polyhedron* link1 = new sf::Polyhedron("ArmLink1", sf::GetDataPath() + "link1_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    sf::Polyhedron* link2 = new sf::Polyhedron("ArmLink2", sf::GetDataPath() + "link2_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    sf::Polyhedron* link3 = new sf::Polyhedron("ArmLink3", sf::GetDataPath() + "link3_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, manipLook);
    sf::Polyhedron* link4 = new sf::Polyhedron("ArmLink4", sf::GetDataPath() + "link4ft_hydro.obj", sf::Scalar(1), sf::Transform::getIdentity(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, link4Look);
    
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
    sf::Polyhedron* prop1 = new sf::Polyhedron("Propeller1", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, propLook);
    sf::Polyhedron* prop2 = new sf::Polyhedron("Propeller2", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, propLook);
    sf::Polyhedron* prop3 = new sf::Polyhedron("Propeller3", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, propLook);
    sf::Polyhedron* prop4 = new sf::Polyhedron("Propeller4", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, propLook);
    sf::Polyhedron* prop5 = new sf::Polyhedron("Propeller5", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Dummy"), sf::BodyPhysicsType::SUBMERGED_BODY, propLook);
    sf::Thruster* thSway = new sf::Thruster("ThrusterSway", prop1, 0.18, 0.48, 0.05, 1000.0, true);
    sf::Thruster* thSurgeP = new sf::Thruster("ThrusterSurgePort", prop2, 0.18, 0.48, 0.05, 1000.0, true);
    sf::Thruster* thSurgeS = new sf::Thruster("ThrusterSurgeStarboard", prop3, 0.18, 0.48, 0.05, 1000.0, true);
    sf::Thruster* thHeaveS = new sf::Thruster("ThrusterHeaveStern", prop4, 0.18, 0.48, 0.05, 1000.0, false);
    sf::Thruster* thHeaveB = new sf::Thruster("ThrusterHeaveBow", prop5, 0.18, 0.48, 0.05, 1000.0, true);
    
    //Create ligths
    //sf::Light* spot1 = new sf::Light("Spot1", sf::Color::BlackBody(4000.0), 100000.0); //OMNI
    //sf::Light* spot1 = new sf::Light("Spot1", 30.0, sf::Color::BlackBody(4000.0), 100000.0);
    
    //Create sensors
    sf::Odometry* odom = new sf::Odometry("Odom");
    sf::Pressure* press = new sf::Pressure("Pressure");
    press->setNoise(1.0);
    sf::DVL* dvl = new sf::DVL("DVL", 30.0);
    dvl->setNoise(0.02, 0.05);
    sf::IMU* imu = new sf::IMU("IMU");
    imu->setNoise(0.01, 0.05);
    sf::FOG* fog = new sf::FOG("FOG");
    fog->setNoise(0.01);
    sf::GPS* gps = new sf::GPS("GPS", 41.77737, 3.03376);
    gps->setNoise(0.5);
    
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
  
    //Sensors
    auv->AddLinkSensor(odom, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0,0,0)));
    auv->AddLinkSensor(press, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0.6,0,-0.7)));
    auv->AddLinkSensor(dvl, "Vehicle", sf::Transform(sf::Quaternion(-M_PI_4,0,M_PI), sf::Vector3(-0.5,0,0.1)));
    auv->AddLinkSensor(imu, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0,0,-0.7)));
    auv->AddLinkSensor(fog, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0.3,0,-0.7)));
    auv->AddLinkSensor(gps, "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.5,0,-0.9)));
    
    AddRobot(auv, sf::Transform(sf::Quaternion(0,0,0.5), sf::Vector3(5,0,1)));
}