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
//  CableTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/11/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include "CableTestManager.h"

#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>
#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/CableEntity.h>
#include <actuators/Light.h>
#include <graphics/OpenGLContent.h>
#include <entities/forcefields/Uniform.h>
#include <entities/solids/Cylinder.h>
#include <actuators/Servo.h>
#include <actuators/Motor.h>
#include <joints/RevoluteJoint.h>
#include <joints/FixedJoint.h>
#include <core/FeatherstoneRobot.h>

CableTestManager::CableTestManager(sf::Scalar stepsPerSecond) 
    : SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void CableTestManager::BuildScenario()
{
    setICSolverParams(false);

    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", sf::UnitSystem::Density(sf::CGS, sf::MKS, 9.81), 0.1);
    CreateMaterial("Wood", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.8), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 1.0, 1.0);
    SetMaterialsInteraction("Ground", "Steel", 1.0, 1.0);
    SetMaterialsInteraction("Ground", "Wood", 1.0, 1.0);
    SetMaterialsInteraction("Steel", "Steel", 1.0, 1.0);
    SetMaterialsInteraction("Steel", "Wood", 1.0, 1.0);
    SetMaterialsInteraction("Wood", "Wood", 1.0, 1.0);
    
    ///////LOOKS///////////
    CreateLook("Grid", sf::Color::Gray(1.f), 0.8f, 0.f, 0.0f, sf::GetShaderPath() + "grid.png");
    CreateLook("Green", sf::Color::RGB(0.1f, 0.8f, 0.1f), 0.5f, 0.0f);
    CreateLook("Red", sf::Color::RGB(1.0f, 0.3f, 0.3f), 0.5f, 0.0f);
    CreateLook("Rope", sf::Color::Gray(1.f), 1.f, 0.f, 0.f, sf::GetDataPath() + "rope_color.jpg", sf::GetDataPath() + "rope_normal.png");

    EnableOcean(0.0);
    getOcean()->setWaterType(0.2);
    getOcean()->AddVelocityField(new sf::Uniform(sf::Vector3(1.0,0.0,0.0)));
    getOcean()->EnableCurrents();
    
    ////////OBJECTS
    sf::Plane* floor = new sf::Plane("Floor", 10000, "Ground", "Grid");
    AddStaticEntity(floor, sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 10.0)));

    sf::BodyPhysicsSettings phy;
    phy.mode = sf::BodyPhysicsMode::SUBMERGED;
    phy.collisions = true;

    // Box attached to a fixed cable
    sf::Box* box = new sf::Box("Box", phy, sf::Vector3(0.1, 0.1, 0.1), sf::I4(), "Steel", "Red");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0.0, 0.0, 2.0)));

    sf::CableEntity* cable = new sf::CableEntity("Cable1", sf::CableEnds::FIRST, sf::Vector3(0, 0, -2), sf::Vector3(0, 0, 1.5), 100, 0.02, "Steel", "Red");
    cable->AttachToSolid(sf::CableEnds::SECOND, box);
    AddEntity(cable);

    // Winch with cable
    sf::Box* box2 = new sf::Box("WinchBase", phy, sf::Vector3(0.1, 0.1, 0.1), sf::I4(), "Steel", "Green");
    sf::Cylinder* cyl = new sf::Cylinder("WinchCylinder", phy, 0.2, 2.0, sf::Transform(sf::Quaternion(0.0, 0.0, M_PI_2), sf::Vector3(0.0, 0.0, 0.0)), "Steel", "Green");

    sf::FeatherstoneRobot* winch = new sf::FeatherstoneRobot("Winch", true);
    winch->DefineLinks(box2, {cyl}, false);
    winch->DefineRevoluteJoint("Joint1", "WinchBase", "WinchCylinder", sf::Transform(sf::IQ(), sf::Vector3(0.0, 1.0, 0.0)), sf::Vector3(0.0, 1.0, 0.0));
    winch->BuildKinematicStructure();
    
    sf::Servo* servo = new sf::Servo("Servo", 1.0, 1.0, 100.0);
    servo->setControlMode(sf::ServoControlMode::VELOCITY);
    winch->AddJointActuator(servo, "Joint1");
    
    AddRobot(winch, sf::Transform(sf::IQ(), sf::Vector3(5.0, 0.0, -5.0)));
    
    sf::Sphere* sphere = new sf::Sphere("CableAttachSphere", phy, 0.01, sf::I4(), "Steel", "Green");
    AddSolidEntity(sphere, sf::Transform(sf::IQ(), sf::Vector3(5.0, 1.0, -4.8)));

    sf::FixedJoint* fixedJoint = new sf::FixedJoint("CableAttachJoint", sphere, winch->getDynamics(), 0);
    AddJoint(fixedJoint);

    sf::CableEntity* cable2 = new sf::CableEntity("Cable2", sf::CableEnds::NONE, sf::Vector3(5.0, 1.0, -4.78), sf::Vector3(5.0, 1.0, -1.0), 100, 0.02, "Steel", "Rope", 80.f);
    cable2->AttachToSolid(sf::CableEnds::FIRST, sphere);
    AddEntity(cable2);
}
