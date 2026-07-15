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
//  Copyright (c) 2025-2026 Patryk Cieslak. All rights reserved.
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
    AddStaticEntity(std::make_unique<sf::Plane>("Floor", 10000.f, "Ground", "Grid"), sf::Transform::getIdentity());

    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::SURFACE;
    phy.collisions = false;
  
#ifdef USE_FEATHERSTONE_ROBOT
    // Robot utilizing the Featherstone's algorithm (higher accuracy and stability)
    std::unique_ptr<sf::FeatherstoneRobot> robot = std::make_unique<sf::FeatherstoneRobot>("Robot", true);
#else    
    // Robot utilizing the general sequential impulse algorithm
    std::unique_ptr<sf::GeneralRobot> robot = std::make_unique<sf::GeneralRobot>("Robot", true);
#endif
    // Mechanical parts
    std::vector<std::unique_ptr<sf::SolidEntity>> links;
    links.push_back(std::make_unique<sf::Box>("Link1", phy, sf::Vector3(0.12,0.12,0.8), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.4)), "Steel", "Green"));
    links.push_back(std::make_unique<sf::Box>("Link2", phy, sf::Vector3(0.1,0.1,0.6), sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.3)), "Steel", "Yellow"));

    // Kinematic chain
    robot->DefineLinks(std::make_unique<sf::Sphere>("Base", phy, 0.1, sf::I4(), "Steel", "Black"), std::move(links), false);
    robot->DefineRevoluteJoint("Joint1", "Base", "Link1", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.0)), sf::Vector3(0,1,0));
    robot->DefineRevoluteJoint("Joint2", "Link1", "Link2", sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 0.8)), sf::Vector3(0,1,0));
    robot->BuildKinematicStructure();

    // Actuators
    robot->AddJointActuator(std::make_unique<sf::Motor>("Motor"), "Joint2");

    // Sensors
    robot->AddJointSensor(std::make_unique<sf::RotaryEncoder>("Encoder1"), "Joint1");
    robot->AddJointSensor(std::make_unique<sf::RotaryEncoder>("Encoder2"), "Joint2");
    
    // Add to simulation
    AddRobot(std::move(robot), sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-2.0)));
}