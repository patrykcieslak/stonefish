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
//  LearningTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/05/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include "LearningTestManager.h"

#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <sensors/Sample.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <core/FeatherstoneRobot.h>
#include <core/GeneralRobot.h>
#include <actuators/Motor.h>

LearningTestManager::LearningTestManager(sf::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, sf::Solver::DANTZIG, sf::CollisionFilter::EXCLUSIVE)
{
}

void LearningTestManager::BuildScenario()
{
    // Introducing some global damping to stabilize the simulation
    sf::Scalar erp, stopErp;
    getJointErp(erp, stopErp);
    setSolverParams(erp, stopErp, 1.0, 0.1, 0.0, 0.0, 0.0);

    // Materials
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.0), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);
    
    // Looks (graphical materials)
    CreateLook("Grid", sf::Color::Gray(1.f), 0.8f, 0.f, 0.0f, sf::GetShaderPath() + "grid.png");
    CreateLook("Black", sf::Color::RGB(0.1f, 0.1f, 0.1f), 0.5f, 0.0f);
    CreateLook("Green", sf::Color::RGB(0.1f, 0.8f, 0.1f), 0.5f, 0.0f);
    CreateLook("Yellow", sf::Color::RGB(1.0f, 0.8f, 0.1f), 0.5f, 0.0f);
    
    // Environment
    sf::Plane* floor = new sf::Plane("Floor", 10000.f, "Ground", "Grid");
    AddStaticEntity(floor, sf::Transform::getIdentity());

    sf::BodyPhysicsSettings phy;
    phy.mode = sf::BodyPhysicsMode::SURFACE;
    phy.collisions = false;

    // Robot   
#ifdef USE_FEATHERSTONE_ROBOT
    // Robot utilizing the Featherstone's algorithm (higher accuracy and stability)
    sf::FeatherstoneRobot* robot = new sf::FeatherstoneRobot("Robot", true);
    
    // Mechanical parts
    sf::Sphere* obj = new sf::Sphere("Base", phy, 0.1, sf::I4(), "Steel", "Black");
    sf::Box* link1 = new sf::Box("Link1", phy, sf::Vector3(0.12,0.12,0.8), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.4)), "Steel", "Green");
    sf::Box* link2 = new sf::Box("Link2", phy, sf::Vector3(0.1,0.1,0.6), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.3)), "Steel", "Yellow");
    std::vector<sf::SolidEntity*> links;
    links.push_back(link1);
    links.push_back(link2);

    // Kinematic chain
    robot->DefineLinks(obj, links, false);
    robot->DefineRevoluteJoint("Joint1", "Base", "Link1", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.0)), sf::Vector3(0,1,0));
    robot->DefineRevoluteJoint("Joint2", "Link1", "Link2", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.8)), sf::Vector3(0,1,0));
#else    
    // Robot utilizing the general sequential impulse algorithm
    sf::GeneralRobot* robot = new sf::GeneralRobot("Robot", true);

    // Mechanical parts
    sf::Sphere* obj = new sf::Sphere("Base", phy, 0.1, sf::I4(), "Steel", "Black");
    sf::Box* link1 = new sf::Box("Link1", phy, sf::Vector3(0.12,0.12,0.8), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.4)), "Steel", "Green");
    sf::Box* link2 = new sf::Box("Link2", phy, sf::Vector3(0.1,0.1,0.6), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.3+0.8)), "Steel", "Yellow");    
    std::vector<sf::SolidEntity*> links;
    links.push_back(link1);
    links.push_back(link2);

    // Kinematic chain
    robot->DefineLinks(obj, links, false);
    robot->DefineRevoluteJoint("Joint1", "Base", "Link1", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.0)), sf::Vector3(0,1,0));
    robot->DefineRevoluteJoint("Joint2", "Link1", "Link2", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.8)), sf::Vector3(0,1,0));
#endif
    robot->BuildKinematicStructure();

    // Actuators
    sf::Motor* motor = new sf::Motor("Motor");
    robot->AddJointActuator(motor, "Joint2");

    // Sensors
    sf::RotaryEncoder* enc1 = new sf::RotaryEncoder("Encoder1");
    sf::RotaryEncoder* enc2 = new sf::RotaryEncoder("Encoder2");
    robot->AddJointSensor(enc1, "Joint1");
    robot->AddJointSensor(enc2, "Joint2");
    
    // Add to simulation
    AddRobot(robot, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-2.0)));
}