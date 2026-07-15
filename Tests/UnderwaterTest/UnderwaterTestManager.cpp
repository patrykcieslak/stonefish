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
//  Copyright(c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include <core/FeatherstoneRobot.h>
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
#include <actuators/Servo.h>
#include <actuators/VariableBuoyancy.h>
#include <sensors/scalar/Pressure.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/scalar/DVL.h>
#include <sensors/scalar/Compass.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/GPS.h>
#include <sensors/Contact.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/Multibeam2.h>
#include <sensors/vision/FLS.h>
#include <sensors/vision/SSS.h>
#include <sensors/vision/MSIS.h>
#include <comms/AcousticModem.h>
#include <sensors/Sample.h>
#include <actuators/Light.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <sensors/scalar/Accelerometer.h>
#include <entities/FeatherstoneEntity.h>
#include <entities/forcefields/Trigger.h>
#include <entities/forcefields/Pipe.h>
#include <entities/forcefields/Jet.h>
#include <entities/forcefields/Uniform.h>
#include <entities/AnimatedEntity.h>
#include <sensors/scalar/Profiler.h>
#include <sensors/scalar/Multibeam.h>
#include <utils/UnitSystem.h>
#include <core/ScenarioParser.h>
#include <core/NED.h>
#include <iomanip>

UnderwaterTestManager::UnderwaterTestManager(sf::Scalar stepsPerSecond)
: SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void UnderwaterTestManager::BuildScenario()
{
#ifdef PARSED_SCENARIO
    sf::ScenarioParser parser(this);
    bool success = parser.Parse(sf::GetDataPath() + "underwater_test.scn");
    parser.SaveLog("underwater_test.log");
    if(!success)
        cCritical("Scenario parser: Parsing failed!");
#else
    ///////MATERIALS////////
    CreateMaterial("Dummy", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.9), 0.3);
    CreateMaterial("Fiberglass", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.5), 0.9);
    CreateMaterial("Rock", sf::UnitSystem::Density(sf::CGS, sf::MKS, 3.0), 0.6);
    SetMaterialsInteraction("Dummy", "Dummy", 0.5, 0.2);
    SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
    SetMaterialsInteraction("Fiberglass", "Dummy", 0.5, 0.2);
    SetMaterialsInteraction("Rock", "Dummy", 0.6, 0.4);
    SetMaterialsInteraction("Rock", "Fiberglass", 0.6, 0.4);
    
    ///////LOOKS///////////
    CreateLook("yellow", sf::Color::RGB(1.f, 0.9f, 0.f), 0.3f, 0.f);
    CreateLook("grey", sf::Color::RGB(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    CreateLook("seabed", sf::Color::RGB(0.7f, 0.7f, 0.5f), 0.9f, 0.f, 0.f, "", sf::GetDataPath() + "sand_normal.png");
    CreateLook("propeller", sf::Color::RGB(1.f, 1.f, 1.f), 0.3f, 0.f, 0.f, sf::GetDataPath() + "propeller_tex.png");
    CreateLook("black", sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.4f, 0.5f);
    CreateLook("manipulator", sf::Color::RGB(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    CreateLook("link4", sf::Color::RGB(1.f, 1.f, 1.f), 0.6f, 0.8f, 0.f, sf::GetDataPath() + "link4_tex.png");
    
    ////////OBJECTS    
    //Create environment
    EnableOcean(0.0);
    getOcean()->setWaterType(0.2);
    getOcean()->AddVelocityField(std::make_unique<sf::Jet>(sf::Vector3(0,0,1.0), sf::VY(), 0.3, 5.0));
    getOcean()->AddVelocityField(std::make_unique<sf::Uniform>(sf::Vector3(1.0,0.0,0.0)));
    getOcean()->EnableCurrents();
    getAtmosphere()->SetSunPosition(0.0, 60.0);
    getNED()->Init(41.77737, 3.03376, 0.0);
    
    AddStaticEntity(std::make_unique<sf::Terrain>("Seabed", sf::GetDataPath() + "terrain.png", 1.0, 1.0, 5.0, "Rock", "seabed", 5.f),
        sf::Transform(sf::IQ(), sf::Vector3(0,0,15.0)));
    AddStaticEntity(std::make_unique<sf::Obstacle>("Cyl", 0.5, 5.0, sf::I4(), "Fiberglass", "seabed"), 
        sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(6.0,2.0,5.0)));
	
	std::unique_ptr<sf::Light> spot = std::make_unique<sf::Light>("Spot", 0.02, 50.0, sf::Color::BlackBody(5000.0), 100.0);
	spot->AttachToWorld(sf::Transform(sf::Quaternion(0,0,M_PI/3.0), sf::Vector3(0.0,0.0,1.0)));
	AddActuator(std::move(spot));
    
    std::unique_ptr<sf::Light> omni = std::make_unique<sf::Light>("Omni", 0.02, sf::Color::BlackBody(5000.0), 10000.0);
	omni->AttachToWorld(sf::Transform(sf::Quaternion(0,0,M_PI/3.0), sf::Vector3(2.0,2.0,0.5)));
	AddActuator(std::move(omni));

    //Create underwater vehicle body
    //Externals
    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::SUBMERGED;
    phy.collisions = true;
    
    phy.buoyancy = false;
    std::unique_ptr<sf::Polyhedron> hullB = std::make_unique<sf::Polyhedron>("HullBottom", phy, sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "yellow", sf::Scalar(0.003));
    std::unique_ptr<sf::Polyhedron> hullP = std::make_unique<sf::Polyhedron>("HullPort", phy, sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "yellow", sf::Scalar(0.003));
    std::unique_ptr<sf::Polyhedron> hullS = std::make_unique<sf::Polyhedron>("HullStarboard", phy, sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "yellow", sf::Scalar(0.003));
    std::unique_ptr<sf::Polyhedron> vBarStern = std::make_unique<sf::Polyhedron>("VBarStern", phy, sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "grey", sf::Scalar(0.003));
    std::unique_ptr<sf::Polyhedron> vBarBow = std::make_unique<sf::Polyhedron>("VBarBow", phy, sf::GetDataPath() + "vbar_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "grey", sf::Scalar(0.003));

    phy.buoyancy = true;
    std::unique_ptr<sf::Polyhedron> ductSway = std::make_unique<sf::Polyhedron>("DuctSway", phy, sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "black");
    std::unique_ptr<sf::Polyhedron> ductSurgeP = std::make_unique<sf::Polyhedron>("DuctSurgePort", phy, sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "black");
    std::unique_ptr<sf::Polyhedron> ductSurgeS = std::make_unique<sf::Polyhedron>("DuctSurgeStarboard", phy, sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "black");
    std::unique_ptr<sf::Polyhedron> ductHeaveS = std::make_unique<sf::Polyhedron>("DuctHeaveStern", phy, sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "black");
    std::unique_ptr<sf::Polyhedron> ductHeaveB = std::make_unique<sf::Polyhedron>("DuctHeaveBow", phy, sf::GetDataPath() + "duct_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "black");
    //Internals
    std::unique_ptr<sf::Cylinder> batteryCyl = std::make_unique<sf::Cylinder>("BatteryCylinder", phy, 0.13, 0.6, sf::I4(), "Dummy", "manipulator");
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(70));
    std::unique_ptr<sf::Cylinder> portCyl = std::make_unique<sf::Cylinder>("PortCylinder", phy, 0.13, 1.0, sf::I4(), "Dummy", "manipulator");
    portCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(20));
    std::unique_ptr<sf::Cylinder> starboardCyl = std::make_unique<sf::Cylinder>("StarboardCylinder", phy, 0.13, 1.0, sf::I4(), "Dummy", "manipulator");
    starboardCyl->ScalePhysicalPropertiesToArbitraryMass(sf::Scalar(20));
    
    //Build whole body
    std::unique_ptr<sf::Compound> vehicle = std::make_unique<sf::Compound>("Vehicle", phy, std::move(hullB), sf::I4());
    vehicle->AddExternalPart(std::move(hullP), sf::Transform(sf::IQ(), sf::Vector3(0,-0.35,-0.7)));
    vehicle->AddExternalPart(std::move(hullS), sf::Transform(sf::IQ(), sf::Vector3(0,0.35,-0.7)));
    vehicle->AddExternalPart(std::move(vBarStern), sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(-0.25,0.0,-0.15)));
    vehicle->AddExternalPart(std::move(vBarBow), sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0.30,0.0,-0.15)));
    vehicle->AddExternalPart(std::move(ductSway), sf::Transform(sf::Quaternion(M_PI_2,M_PI,0), sf::Vector3(-0.0137, 0.0307, -0.38)));
    vehicle->AddExternalPart(std::move(ductSurgeP), sf::Transform(sf::Quaternion(0,0,M_PI), sf::Vector3(-0.2807,-0.2587,-0.38)));
    vehicle->AddExternalPart(std::move(ductSurgeS), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(-0.2807,0.2587,-0.38)));
    vehicle->AddExternalPart(std::move(ductHeaveS), sf::Transform(sf::Quaternion(M_PI_2,-M_PI_2,0), sf::Vector3(-0.5337,0.0,-0.6747)));
    vehicle->AddExternalPart(std::move(ductHeaveB), sf::Transform(sf::Quaternion(-M_PI_2,-M_PI_2,0), sf::Vector3(0.5837,0.0,-0.6747)));
    vehicle->AddInternalPart(std::move(batteryCyl), sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(-0.1,0,0)));
    vehicle->AddInternalPart(std::move(portCyl), sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,-0.35,-0.7)));
    vehicle->AddInternalPart(std::move(starboardCyl), sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,0.35,-0.7)));
    
    vehicle->setDisplayInternalParts(false);
    
    //Manipulator bodies
    std::unique_ptr<sf::Polyhedron> baseLink = std::make_unique<sf::Polyhedron>("ArmBaseLink", phy, sf::GetDataPath() + "base_link_uji_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "manipulator");
    std::unique_ptr<sf::Polyhedron> link1 = std::make_unique<sf::Polyhedron>("ArmLink1", phy, sf::GetDataPath() + "link1_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "manipulator");
    std::unique_ptr<sf::Polyhedron> link2 = std::make_unique<sf::Polyhedron>("ArmLink2", phy, sf::GetDataPath() + "link2_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "manipulator");
    std::unique_ptr<sf::Polyhedron> link3 = std::make_unique<sf::Polyhedron>("ArmLink3", phy, sf::GetDataPath() + "link3_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "manipulator");
    std::unique_ptr<sf::Polyhedron> link4 = std::make_unique<sf::Polyhedron>("ArmLink4", phy, sf::GetDataPath() + "link4ft_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "link4");
    std::unique_ptr<sf::Polyhedron> ee = std::make_unique<sf::Polyhedron>("EE", phy, sf::GetDataPath() + "eeprobe_hydro.obj", sf::Scalar(1), sf::I4(), "Neutral", "manipulator");
    std::unique_ptr<sf::Polyhedron> finger1 = std::make_unique<sf::Polyhedron>("Finger1", phy, sf::GetDataPath() + "fingerA_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "manipulator");
    std::unique_ptr<sf::Polyhedron> finger2 = std::make_unique<sf::Polyhedron>("Finger2", phy, sf::GetDataPath() + "fingerA_hydro.obj", sf::Scalar(1), sf::I4(), "Dummy", "manipulator");
    
    std::vector<std::unique_ptr<sf::SolidEntity>> arm;
    arm.push_back(std::move(baseLink));
    arm.push_back(std::move(link1));
    arm.push_back(std::move(link2));
    arm.push_back(std::move(link3));
    arm.push_back(std::move(link4));
    arm.push_back(std::move(ee));
    arm.push_back(std::move(finger1));
    arm.push_back(std::move(finger2));
    
    //Create manipulator servomotors
    std::unique_ptr<sf::Servo> srv1 = std::make_unique<sf::Servo>("Servo1", 1.0, 1.0, 100.0);
    std::unique_ptr<sf::Servo> srv2 = std::make_unique<sf::Servo>("Servo2", 1.0, 1.0, 100.0);
    std::unique_ptr<sf::Servo> srv3 = std::make_unique<sf::Servo>("Servo3", 1.0, 1.0, 100.0);
    std::unique_ptr<sf::Servo> srv4 = std::make_unique<sf::Servo>("Servo4", 1.0, 1.0, 100.0);
    std::unique_ptr<sf::Servo> srv5 = std::make_unique<sf::Servo>("FServo1", 1.0, 1.0, 10.0);
    std::unique_ptr<sf::Servo> srv6 = std::make_unique<sf::Servo>("FServo2", 1.0, 1.0, 10.0);
    
    //Create thrusters
    std::array<std::string, 5> thrusterNames = {"ThrusterSway", "ThrusterSurgePort", "ThrusterSurgeStarboard", "ThrusterHeaveStern", "ThrusterHeaveBow"}; 
    std::array<std::unique_ptr<sf::Thruster>, 5> thrusters;
    for(size_t i=0; i<thrusterNames.size(); ++i)
    {
        std::unique_ptr<sf::Polyhedron> propeller = std::make_unique<sf::Polyhedron>("Propeller", phy, sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), "Dummy", "propeller");
        std::unique_ptr<sf::MechanicalPI> rotorDynamics;
        rotorDynamics = std::make_unique<sf::MechanicalPI>(1.0, 10.0, 5.0, 5.0);
        std::unique_ptr<sf::FDThrust> thrustModel;
        thrustModel = std::make_unique<sf::FDThrust>(0.18, 0.48, 0.48, 0.05, true, getOcean()->getLiquid().density);
        thrusters[i] = std::make_unique<sf::Thruster>(thrusterNames[i], std::move(propeller), std::move(rotorDynamics), std::move(thrustModel), 0.18, true, 105.0, true, true);
    }

    //Create VBS
    //std::vector<std::string> vmeshes;
    //vmeshes.push_back(sf::GetDataPath() + "vbs_max.obj");
    //vmeshes.push_back(sf::GetDataPath() + "vbs_min.obj");
    //std::unique_ptr<VariableBuoyancy> vbs = std::make_unique<sf::VariableBuoyancy>("VBS", vmeshes, 0.002);
       
    //Create sensors
    std::unique_ptr<sf::Odometry> odom = std::make_unique<sf::Odometry>("Odom");
    
    std::unique_ptr<sf::Pressure> press = std::make_unique<sf::Pressure>("Pressure");
    press->setNoise(1.0);
    
    std::unique_ptr<sf::DVL> dvl = std::make_unique<sf::DVL>("DVL", 30.0, false);
    dvl->setNoise(0.0, 0.02, 0.05, 0.0, 0.02);
    
    std::unique_ptr<sf::IMU> imu = std::make_unique<sf::IMU>("IMU");
    imu->setNoise(sf::V0(), sf::Vector3(0.05, 0.05, 0.1), 0.0, sf::Vector3(0.01, 0.01, 0.02));
    
    std::unique_ptr<sf::Compass> fog = std::make_unique<sf::Compass>("FOG");
    fog->setNoise(0.01);
    
    std::unique_ptr<sf::GPS> gps = std::make_unique<sf::GPS>("GPS");
    gps->setNoise(0.5);
    
    //std::unique_ptr<sf::Multibeam2> mb = std::make_unique<sf::Multibeam2>("Multibeam", 1000, 300, 50.0, 40.0, 0.1, 10.0, 10.0);
    //mb->setDisplayOnScreen(true);
    
    //std::unique_ptr<sf::DepthCamera> dc = std::make_unique<sf::DepthCamera>("DepthCam", 1000, 350, 50.0, 0.1, 10.0, 10.0);
    //dc->setDisplayOnScreen(true);
    
    std::unique_ptr<sf::FLS> fls = std::make_unique<sf::FLS>("FLS", 256, 500, 150.0, 30.0, 1.0, 20.0, sf::ColorMap::GREEN_BLUE, sf::SonarOutputFormat::U8);
    fls->setNoise(0.05, 0.05);
    fls->setDisplayOnScreen(true, 800, 250, 0.4f);
    //fls->InstallNewDataHandler(std::bind(&UnderwaterTestManager::FLSDataCallback, this, std::placeholders::_1));

    std::unique_ptr<sf::MSIS> msis = std::make_unique<sf::MSIS>("MSIS", 1.5, 500, 2.0, 30.0, -50, 50, 1.0, 100.0, sf::ColorMap::GREEN_BLUE, sf::SonarOutputFormat::U8);
    msis->setDisplayOnScreen(true, 880, 455, 0.6f);
    //msis->InstallNewDataHandler(std::bind(&UnderwaterTestManager::MSISDataCallback, this, std::placeholders::_1));
    
    std::unique_ptr<sf::SSS> sss = std::make_unique<sf::SSS>("SSS", 800, 400, 70.0, 1.5, 50.0, 1.0, 100.0, sf::ColorMap::GREEN_BLUE, sf::SonarOutputFormat::U8);
    sss->setDisplayOnScreen(true, 710, 5, 0.6f);
    //sss->InstallNewDataHandler(std::bind(&UnderwaterTestManager::SSSDataCallback, this, std::placeholders::_1));
    
    //std::unique_ptr<sf::ColorCamera> cam = std::make_unique<sf::ColorCamera>("Cam", 300, 200, 60.0, 10.0);
    //cam->setDisplayOnScreen(true);
    
    //std::unique_ptr<sf::ColorCamera> cam2 = std::make_unique<sf::ColorCamera>("Cam", 300, 200, 60.0);
    
    //Create AUV
    std::unique_ptr<sf::Robot> auv = std::make_unique<sf::FeatherstoneRobot>("GIRONA500", false);
    
    //Mechanical structure
    auv->DefineLinks(std::move(vehicle), std::move(arm));
    auv->DefineFixedJoint("VehicleToArm", "Vehicle", "ArmBaseLink", sf::Transform(sf::IQ(), sf::Vector3(0.74,0.0,0.0)));
    auv->DefineRevoluteJoint("Joint1", "ArmBaseLink", "ArmLink1", sf::I4(), sf::Vector3(0.0, 0.0, 1.0), std::make_pair(-1.0, 1.0));
    auv->DefineRevoluteJoint("Joint2", "ArmLink1", "ArmLink2", sf::Transform(sf::IQ(), sf::Vector3(0.1065, 0.0, 0.0)), sf::Vector3(0.0, 1.0, 0.0), std::make_pair(-1.0, 1.0));
    auv->DefineRevoluteJoint("Joint3", "ArmLink2", "ArmLink3", sf::Transform(sf::IQ(), sf::Vector3(0.23332, 0.0, 0.0)), sf::Vector3(0.0, 1.0, 0.0), std::make_pair(-1.0, 1.0));
    auv->DefineRevoluteJoint("Joint4", "ArmLink3", "ArmLink4", sf::Transform(sf::IQ(), sf::Vector3(0.103, 0.0, 0.201)), sf::Vector3(0.0, 0.0, 1.0),  std::make_pair(-1.0, 1.0));
    auv->DefineFixedJoint("Fix", "ArmLink4", "EE", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.05)));
    auv->DefineRevoluteJoint("Joint5", "EE", "Finger1", sf::Transform(sf::IQ(), sf::Vector3(0.03,0,0.1)), sf::VY(), std::make_pair(0.0, 1.0));
    auv->DefineRevoluteJoint("Joint6", "EE", "Finger2", sf::Transform(sf::Quaternion(M_PI, 0.0, 0.0), sf::Vector3(-0.03,0,0.1)), sf::VY(), std::make_pair(0.0, 1.0));
    auv->BuildKinematicStructure();
    
    //Joint motors
    auv->AddJointActuator(std::move(srv1), "Joint1");
    auv->AddJointActuator(std::move(srv2), "Joint2");
    auv->AddJointActuator(std::move(srv3), "Joint3");
    auv->AddJointActuator(std::move(srv4), "Joint4");
    auv->AddJointActuator(std::move(srv5), "Joint5");
    auv->AddJointActuator(std::move(srv6), "Joint6");
    
    //Thrusters
    auv->AddLinkActuator(std::move(thrusters[0]), "Vehicle", sf::Transform(sf::Quaternion(M_PI_2,M_PI,0), sf::Vector3(-0.0137, 0.0307, -0.38)));
    auv->AddLinkActuator(std::move(thrusters[1]), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.2807,-0.2587,-0.38)));
    auv->AddLinkActuator(std::move(thrusters[2]), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.2807,0.2587,-0.38)));
    auv->AddLinkActuator(std::move(thrusters[3]), "Vehicle", sf::Transform(sf::Quaternion(0,-M_PI_2,0), sf::Vector3(-0.5337,0.0,-0.6747)));
    auv->AddLinkActuator(std::move(thrusters[4]), "Vehicle", sf::Transform(sf::Quaternion(0,-M_PI_2,0), sf::Vector3(0.5837,0.0,-0.6747)));
    //auv->AddLinkActuator(std::move(vbs), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.5,0.0,0.0)));
    
    //Sensors
    auv->AddLinkSensor(std::move(odom), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0,0,0)));
    auv->AddLinkSensor(std::move(press), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0.6,0,-0.7)));
    auv->AddLinkSensor(std::move(dvl), "Vehicle", sf::Transform(sf::Quaternion(-M_PI_4,0,M_PI), sf::Vector3(-0.5,0,0.1)));
    auv->AddLinkSensor(std::move(imu), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0,0,-0.7)));
    auv->AddLinkSensor(std::move(fog), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(0.3,0,-0.7)));
    auv->AddLinkSensor(std::move(gps), "Vehicle", sf::Transform(sf::IQ(), sf::Vector3(-0.5,0,-0.9)));
    auv->AddVisionSensor(std::move(fls), "Vehicle", sf::Transform(sf::Quaternion(1.57, 0.0, 0.8), sf::Vector3(0.0,0.0,1.0)));
    auv->AddVisionSensor(std::move(sss), "Vehicle", sf::Transform(sf::Quaternion(1.57, 0.0, 0.0), sf::Vector3(0.0,0.0,0.0)));
    auv->AddVisionSensor(std::move(msis), "Vehicle", sf::Transform(sf::Quaternion(0.0, 0.0, 1.57), sf::Vector3(0.0,0.0,1.0)));
    //auv->AddVisionSensor(std::move(cam), "Vehicle", sf::Transform(sf::Quaternion(1.57, 0.0, 1.57), sf::Vector3(0.0,0.0,1.0)));
    //auv->AddVisionSensor(std::move(cam2), "Vehicle", sf::Transform(sf::Quaternion(1.57, 0.0, 1.57), sf::Vector3(0.0,0.0,2.0)));
    AddRobot(std::move(auv), sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(0.0,0.0,2.0)));
#endif
} 

void UnderwaterTestManager::SimulationStepCompleted(sf::Scalar timeStep)
{
    if(false)
    {
#ifdef PARSED_SCENARIO
        sf::Thruster* th = dynamic_cast<sf::Thruster*>(getRobot("GIRONA500")->getActuator("GIRONA500/ThrusterSurgePort"));
#else
        sf::Thruster* th = dynamic_cast<sf::Thruster*>(getRobot("GIRONA500")->getActuator("ThrusterSurgePort"));
#endif
        if (th)
        {
            double rpm = th->getOmega() * sf::Scalar(60.0) / sf::Scalar(2.0 * M_PI);
            std::cout << std::setprecision(3) << "[" << th->getName() << "] RPM: " << rpm << ", Thrust: " << th->getThrust() << std::endl;
        }
    }
}

void UnderwaterTestManager::FLSDataCallback(sf::FLS* fls)
{
    auto format = fls->getOutputFormat();
    switch (format)
    {
        case sf::SonarOutputFormat::U8:
            PrintData<GLubyte>(fls->getImageDataPointer(), 10);
            break;

        case sf::SonarOutputFormat::U16:
            PrintData<GLushort>(fls->getImageDataPointer(), 10);
            break;

        case sf::SonarOutputFormat::U32:
            PrintData<GLuint>(fls->getImageDataPointer(), 10);
            break;

        case sf::SonarOutputFormat::F32:
            PrintData<GLfloat>(fls->getImageDataPointer(), 10);
            break;
    }
    std::cout << std::endl;
}

void UnderwaterTestManager::MSISDataCallback(sf::MSIS* msis)
{
    auto format = msis->getOutputFormat();
    switch (format)
    {
        case sf::SonarOutputFormat::U8:
            PrintData<GLubyte>(msis->getImageDataPointer(), 1000);
            break;

        case sf::SonarOutputFormat::U16:
            PrintData<GLushort>(msis->getImageDataPointer(), 1000);
            break;

        case sf::SonarOutputFormat::U32:
            PrintData<GLuint>(msis->getImageDataPointer(), 1000);
            break;

        case sf::SonarOutputFormat::F32:
            PrintData<GLfloat>(msis->getImageDataPointer(), 1000);
            break;
    }
    std::cout << std::endl;
}

void UnderwaterTestManager::SSSDataCallback(sf::SSS* sss)
{
    auto format = sss->getOutputFormat();
    switch (format)
    {
        case sf::SonarOutputFormat::U8:
            PrintData<GLubyte>(sss->getImageDataPointer(), 100);
            break;

        case sf::SonarOutputFormat::U16:
            PrintData<GLushort>(sss->getImageDataPointer(), 100);
            break;

        case sf::SonarOutputFormat::U32:
            PrintData<GLuint>(sss->getImageDataPointer(), 100);
            break;

        case sf::SonarOutputFormat::F32:
            PrintData<GLfloat>(sss->getImageDataPointer(), 100);
            break;
    }
    std::cout << std::endl;
}
