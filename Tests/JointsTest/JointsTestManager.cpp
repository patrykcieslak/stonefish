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
//  JointsTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2023 Patryk Cieslak. All rights reserved.
//

#include "JointsTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Polyhedron.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLTrackball.h>
#include <joints/FixedJoint.h>
#include <joints/SphericalJoint.h>
#include <joints/RevoluteJoint.h>
#include <joints/PrismaticJoint.h>
#include <joints/CylindricalJoint.h>
#include <actuators/SuctionCup.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/GeneralRobot.h>
#include <actuators/Servo.h>
#include <sensors/scalar/GPS.h>
#include <core/NED.h>

JointsTestManager::JointsTestManager(sf::Scalar stepsPerSecond) 
  : SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void JointsTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Steel", sf::UnitSystem::Density(sf::UnitSystems::CGS, sf::UnitSystems::MKS, 7.8), 0.5);
    CreateMaterial("Plastic", sf::UnitSystem::Density(sf::UnitSystems::CGS, sf::UnitSystems::MKS, 1.5), 0.2);
    SetMaterialsInteraction("Steel", "Plastic", 0.8, 0.2);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.1);
    SetMaterialsInteraction("Plastic", "Plastic", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("grid", sf::Color(1.f, 1.f, 1.f), 0.5f, 0.0f, 0.0f, sf::GetShaderPath() + "grid.png");
    CreateLook("orange", sf::Color(1.0f,0.6f,0.3f), 0.3f);
    CreateLook("green", sf::Color(0.5f,1.0f,0.4f), 0.5f);
    
    ////////OBJECTS
    setSolverParams(0.25, 0.5, 0.25, 0.25, 0.0, -1.0, -1.0);
    getAtmosphere()->SetSunPosition(0.0, 70.0);
    static_cast<sf::GraphicalSimulationApp*>(sf::SimulationApp::getApp())->getTrackball()->MoveCenter(glm::vec3(1.f,3.f,0.f));
    getNED()->Init(-10.0, -10.0, 0.0);
    
    std::unique_ptr<sf::Plane> floor = std::make_unique<sf::Plane>("Floor", 1000.f, "Steel", "grid");
    AddStaticEntity(std::move(floor), sf::I4());
    
    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::SURFACE;
    phy.collisions = true;

    //----Fixed Joint----
    sf::SolidEntity* box = AddSolidEntity(
        std::make_unique<sf::Box>("Box", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic","green"), 
        sf::Transform(sf::IQ(), sf::Vector3(-2.0,0.0,-3.0))
    );
    
    sf::SolidEntity* sph = AddSolidEntity(
        std::make_unique<sf::Sphere>("Sph1", phy, sf::Scalar(0.2), sf::I4(), "Steel", "orange"), 
        sf::Transform(sf::IQ(), sf::Vector3(-2.0,-0.5,-3.0))
    );
    AddJoint(std::make_unique<sf::FixedJoint>("Fix", box, sph));
    
    //----Revolute Joint----
    box = AddSolidEntity(
        std::make_unique<sf::Box>("Box1", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "green"), 
        sf::Transform(sf::IQ(), sf::Vector3(-2.5,0.0,-1.0))
    );
    
    sf::SolidEntity* box2 = AddSolidEntity(
        std::make_unique<sf::Box>("Box2", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "orange"), 
        sf::Transform(sf::IQ(), sf::Vector3(-2.5,0.2,-1.0))
    );
    AddJoint(std::make_unique<sf::RevoluteJoint>("Revolute", box, box2, sf::Vector3(-2.5,0.1,-0.9), sf::Vector3(0,1,0), false));
    
    sph = AddSolidEntity(
        std::make_unique<sf::Sphere>("Sph2", phy, sf::Scalar(0.2), sf::I4(), "Plastic", "green"),
        sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-3.0))
    );
    AddJoint(std::make_unique<sf::RevoluteJoint>("RevoluteFix", sph, sf::Vector3(0.0,1.0,-3.0), sf::Vector3(1.0,0.0,0.0)));
    
    //----Spherical Joint----
    sph = AddSolidEntity(
        std::make_unique<sf::Sphere>("Sph3", phy, sf::Scalar(0.2), sf::I4(), "Plastic", "green"),
        sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.0, -1.0))
    );
    sf::SolidEntity* sph2 = AddSolidEntity(
        std::make_unique<sf::Sphere>("Sph4", phy, sf::Scalar(0.15), sf::I4(), "Plastic", "orange"),
        sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.2, -0.4))
    );
    AddJoint(std::make_unique<sf::SphericalJoint>("Spherical", sph, sph2, sf::Vector3(0.0, -2.0, -0.6)));
    
    //----Prismatic Joint----
    box = AddSolidEntity(
        std::make_unique<sf::Box>("Box4", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "green"),
        sf::Transform(sf::IQ(), sf::Vector3(2.0,0.0,-0.051))
    );
    
    box2 = AddSolidEntity(
        std::make_unique<sf::Box>("Box5", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "orange"),
        sf::Transform(sf::IQ(), sf::Vector3(2.0,0.0,-0.5))
    );
    AddJoint(std::make_unique<sf::PrismaticJoint>("Prismatic", box, box2, sf::Vector3(0.5,0,-1.0)));
    
    // //----Cylindrical Joint----
    box = AddSolidEntity(
        std::make_unique<sf::Box>("Box6", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "green"),
        sf::Transform(sf::IQ(), sf::Vector3(-1.0,0.0,-0.051))
    );
    
    box2 = AddSolidEntity(
        std::make_unique<sf::Box>("Box7", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "orange"),
        sf::Transform(sf::IQ(), sf::Vector3(-1.0,0.0,-0.5))
    );
    
    AddJoint(std::make_unique<sf::CylindricalJoint>("Cylindrical", box, box2, sf::Vector3(-1.0, 0.050, -0.25), sf::Vector3(0,0,1)));

    //----General robot
    std::unique_ptr<sf::GeneralRobot> robot = std::make_unique<sf::GeneralRobot>("Manipulator", true);
    
    // Base link
    std::unique_ptr<sf::Box> base = std::make_unique<sf::Box>("Base", phy, sf::Vector3(0.1,0.1,0.1), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0, 0.0)), "Plastic","green");
    // Arm
    std::unique_ptr<sf::Box> arm1 = std::make_unique<sf::Box>("Arm1", phy, sf::Vector3(0.1,0.1,0.5), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0, -0.25)), "Plastic","green");
    std::unique_ptr<sf::Box> arm2 = std::make_unique<sf::Box>("Arm2", phy, sf::Vector3(0.8,0.1,0.1), sf::Transform(sf::IQ(), sf::Vector3(0.4, 0, -0.5)), "Plastic","green");
    std::unique_ptr<sf::Box> arm3 = std::make_unique<sf::Box>("Arm3", phy, sf::Vector3(0.1,0.1,0.5), sf::Transform(sf::IQ(), sf::Vector3(0.8, 0, -0.25)), "Plastic","green");

    std::vector<std::unique_ptr<sf::SolidEntity>> links;
    links.push_back(std::move(arm1));
    links.push_back(std::move(arm2));
    links.push_back(std::move(arm3));
    robot->DefineLinks(std::move(base), std::move(links));
    
    robot->DefineRevoluteJoint("joint1", "Base", "Arm1", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.0)), sf::Vector3(0, 0, 1.0));
    robot->DefineRevoluteJoint("joint2", "Arm1", "Arm2", sf::Transform(sf::IQ(), sf::Vector3(0, 0.0, -0.5)), sf::Vector3(0, 1.0, 0.0));
    robot->DefineRevoluteJoint("joint3", "Arm2", "Arm3", sf::Transform(sf::IQ(), sf::Vector3(0.8, 0.0, -0.5)), sf::Vector3(0, 1.0, 0.0));
    
    robot->BuildKinematicStructure();

    sf::Servo* srv1 = new sf::Servo("Servo1", 1.0, 1.0, 1000.0);
    srv1->setControlMode(sf::ServoControlMode::POSITION);
    robot->AddJointActuator(srv1, "joint1");

    sf::Servo* srv2 = new sf::Servo("Servo2", 1.0, 1.0, 1000.0);
    srv2->setControlMode(sf::ServoControlMode::POSITION);
    robot->AddJointActuator(srv2, "joint2");

    sf::Servo* srv3 = new sf::Servo("Servo3", 1.0, 1.0, 1000.0);
    srv3->setControlMode(sf::ServoControlMode::POSITION);
    robot->AddJointActuator(srv3, "joint3");

    sf::SuctionCup* suction = new sf::SuctionCup("Suction");
    robot->AddLinkActuator(suction, "Arm3", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.0)));

    AddRobot(std::move(robot), sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-0.75)));

    std::unique_ptr<sf::Box> tallbox = std::make_unique<sf::Box>("BoxBig", phy, sf::Vector3(0.2,0.2,0.6), sf::I4(), "Plastic","green");
    tallbox->ScalePhysicalPropertiesToArbitraryMass(1.0);
    AddSolidEntity(std::move(tallbox), sf::Transform(sf::IQ(), sf::Vector3(0.7, 0.0, -0.3) ));
}
