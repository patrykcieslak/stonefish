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
//  FlyingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "FlyingTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <entities/solids/Compound.h>
#include <entities/solids/Box.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/FeatherstoneRobot.h>
#include <sensors/scalar/Odometry.h>
#include <entities/statics/Plane.h>
#include <actuators/Propeller.h>

FlyingTestManager::FlyingTestManager(sf::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void FlyingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Fiberglass", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.5), 0.3);
    CreateMaterial("Ground", 1000.0, 1.0);
    SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    SetMaterialsInteraction("Fiberglass", "Ground", 0.7, 0.4);
    SetMaterialsInteraction("Ground", "Ground", 0.9, 0.7);
    
    ///////LOOKS///////////
    CreateLook("grid", sf::Color::Gray(1.f), 0.8f, 0.f, 0.f, sf::GetShaderPath() + "grid.png");
    CreateLook("white", sf::Color::Gray(1.f), 0.9f, 0.0f, 0.f);
    CreateLook("propeller", sf::Color::RGB(1.f, 0.f, 0.f), 0.3f, 0.f, 0.f);

    ////////OBJECTS    
    //Create environment
    getAtmosphere()->SetSunPosition(0.0, 60.0);
    
    AddStaticEntity(std::make_unique<sf::Plane>("Floor", 10000, "Ground", "grid"), sf::Transform::getIdentity());

    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::AERODYNAMIC;
    phy.collisions = true;
    AddSolidEntity(std::make_unique<sf::Box>("Leaf", phy, sf::Vector3(5,3,0.0001), sf::I4(), "Fiberglass","white"), sf::Transform(sf::Quaternion(0,0.3,0), sf::Vector3(10,0,-10)));
    
    std::unique_ptr<sf::Compound> fuselage = std::make_unique<sf::Compound>("Fuselage", phy, std::make_unique<sf::Box>("Arm1", phy, sf::Vector3(1.0,0.01,0.01), sf::I4(), "Fiberglass", "white"), sf::I4());
    fuselage->AddExternalPart(std::make_unique<sf::Box>("Arm2", phy, sf::Vector3(0.01,1.0,0.01), sf::I4(), "Fiberglass", "white"), sf::I4());

    std::unique_ptr<sf::FeatherstoneRobot> quadCopter = std::make_unique<sf::FeatherstoneRobot>("Quadcopter");
    quadCopter->DefineLinks(std::move(fuselage));
    quadCopter->BuildKinematicStructure();
    quadCopter->AddLinkActuator(std::make_unique<sf::Propeller>("Propeller1", std::make_unique<sf::Polyhedron>("Prop1", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller"), 
        0.2, 0.1, 0.01, 10000, true), "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.5,0.0,-0.02)));
    quadCopter->AddLinkActuator(std::make_unique<sf::Propeller>("Propeller2", std::make_unique<sf::Polyhedron>("Prop2", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller"), 
        0.2, 0.1, 0.01, 10000, true), "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(-0.5,0.0,-0.02)));
    quadCopter->AddLinkActuator(std::make_unique<sf::Propeller>("Propeller3", std::make_unique<sf::Polyhedron>("Prop3", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller"), 
        0.2, 0.1, 0.01, 10000, false), "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,0.5,-0.02)));
    quadCopter->AddLinkActuator(std::make_unique<sf::Propeller>("Propeller4", std::make_unique<sf::Polyhedron>("Prop4", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller"), 
        0.2, 0.1, 0.01, 10000, false), "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,-0.5,-0.02)));
    
    sf::Robot* robot = AddRobot(std::move(quadCopter), sf::Transform(sf::IQ(), sf::Vector3(0,0,-0.02)));
    static_cast<sf::Propeller*>(robot->getActuator("Propeller1"))->setSetpoint(0.5);
    static_cast<sf::Propeller*>(robot->getActuator("Propeller2"))->setSetpoint(0.5);
    static_cast<sf::Propeller*>(robot->getActuator("Propeller3"))->setSetpoint(-0.5);
    static_cast<sf::Propeller*>(robot->getActuator("Propeller4"))->setSetpoint(-0.5);
}