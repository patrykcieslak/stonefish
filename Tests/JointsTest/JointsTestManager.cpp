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
  : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE)
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
    getAtmosphere()->SetupSunPosition(0.0, 70.0);
    getTrackball()->MoveCenter(glm::vec3(1.f,3.f,0.f));
    getNED()->Init(-10.0, -10.0, 0.0);
    
    sf::Plane* floor = new sf::Plane("Floor", 1000.f, "Steel", "grid");
    AddStaticEntity(floor, sf::I4());
    
    sf::BodyPhysicsSettings phy;
    phy.mode = sf::BodyPhysicsMode::SURFACE;
    phy.collisions = true;

    //----Fixed Joint----
    sf::Box* box = new sf::Box("Box", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic","green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(-2.0,0.0,-3.0)));
    
    sf::Sphere* sph = new sf::Sphere("Sph1", phy, sf::Scalar(0.2), sf::I4(), "Steel", "orange");
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(-2.0,-0.5,-3.0)));
    
    sf::FixedJoint* fixed = new sf::FixedJoint("Fix", box, sph);
    AddJoint(fixed);
    
    //----Revolute Joint----
    box = new sf::Box("Box1", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(-2.5,0.0,-1.0)));
    
    sf::Box* box2 = new sf::Box("Box2", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "orange");
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(-2.5,0.2,-1.0)));
    
    sf::RevoluteJoint* revo = new sf::RevoluteJoint("Revolute", box, box2, sf::Vector3(-2.5,0.1,-0.9), sf::Vector3(0,1,0), false);
    AddJoint(revo);
    
    sph = new sf::Sphere("Sph2", phy, sf::Scalar(0.2), sf::I4(), "Plastic", "green");
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-3.0)));
    
    revo = new sf::RevoluteJoint("RevoluteFix", sph, sf::Vector3(0.0,1.0,-3.0), sf::Vector3(1.0,0.0,0.0));
    AddJoint(revo);
    
    //----Spherical Joint----
    sph = new sf::Sphere("Sph3", phy, sf::Scalar(0.2), sf::I4(), "Plastic", "green");
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.0, -1.0)));
    sf::Sphere* sph2 = new sf::Sphere("Sph4", phy, sf::Scalar(0.15), sf::I4(), "Plastic", "orange");
    AddSolidEntity(sph2, sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.2, -0.4)));
    
    sf::SphericalJoint* spher = new sf::SphericalJoint("Spherical", sph, sph2, sf::Vector3(0.0, -2.0, -0.6));
    AddJoint(spher);
    
    //----Prismatic Joint----
    box = new sf::Box("Box4", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(2.0,0.0,-0.051)));
    
    box2 = new sf::Box("Box5", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "orange");
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(2.0,0.0,-0.5)));
    
    sf::PrismaticJoint* trans = new sf::PrismaticJoint("Prismatic", box, box2, sf::Vector3(0.5,0,-1.0));
    AddJoint(trans);
    
    // //----Cylindrical Joint----
    box = new sf::Box("Box6", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(-1.0,0.0,-0.051)));
    
    box2 = new sf::Box("Box7", phy, sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", "orange");
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(-1.0,0.0,-0.5)));
    
    sf::CylindricalJoint* cyli = new sf::CylindricalJoint("Cylindrical", box, box2, sf::Vector3(-1.0, 0.050, -0.25), sf::Vector3(0,0,1));
    AddJoint(cyli);

    //----General robot
    sf::GeneralRobot* robot = new sf::GeneralRobot("Manipulator", true);
    
    // Base link
    sf::Box* base = new sf::Box("Base", phy, sf::Vector3(0.1,0.1,0.1), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0, 0.0)), "Plastic","green");
    // Arm
    sf::Box* arm1 = new sf::Box("Arm1", phy, sf::Vector3(0.1,0.1,0.5), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0, -0.25)), "Plastic","green");
    sf::Box* arm2 = new sf::Box("Arm2", phy, sf::Vector3(0.8,0.1,0.1), sf::Transform(sf::IQ(), sf::Vector3(0.4, 0, -0.5)), "Plastic","green");
    sf::Box* arm3 = new sf::Box("Arm3", phy, sf::Vector3(0.1,0.1,0.5), sf::Transform(sf::IQ(), sf::Vector3(0.8, 0, -0.25)), "Plastic","green");

    std::vector<sf::SolidEntity*> links;
    links.push_back(arm1);
    links.push_back(arm2);
    links.push_back(arm3);
    robot->DefineLinks(base, links);
    
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

    AddRobot(robot, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-0.75)));

    sf::Box* tallbox = new sf::Box("BoxBig", phy, sf::Vector3(0.2,0.2,0.6), sf::I4(), "Plastic","green");
    tallbox->ScalePhysicalPropertiesToArbitraryMass(1.0);
    AddSolidEntity(tallbox, sf::Transform(sf::IQ(), sf::Vector3(0.7, 0.0, -0.3) ));
}
