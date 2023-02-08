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
//  Copyright (c) 2019-2023 Patryk Cieslak. All rights reserved.
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
   : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE)
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
    getAtmosphere()->SetupSunPosition(0.0, 60.0);
    
    sf::Plane* floor = new sf::Plane("Floor", 10000, "Ground", "grid");
    AddStaticEntity(floor, sf::Transform::getIdentity());

    sf::BodyPhysicsSettings phy;
    phy.mode = sf::BodyPhysicsMode::AERODYNAMIC;
    phy.collisions = true;

    sf::Box* leaf = new sf::Box("Leaf", phy, sf::Vector3(5,3,0.0001), sf::I4(), "Fiberglass","white");
    AddSolidEntity(leaf, sf::Transform(sf::Quaternion(0,0.3,0), sf::Vector3(10,0,-10)));
    
    sf::Polyhedron* propeller1 = new sf::Polyhedron("Propeller1", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller");
    sf::Polyhedron* propeller2 = new sf::Polyhedron("Propeller2", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller");
    sf::Polyhedron* propeller3 = new sf::Polyhedron("Propeller3", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller");
    sf::Polyhedron* propeller4 = new sf::Polyhedron("Propeller4", phy, sf::GetDataPath() + "propeller_air.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller");
    
    sf::Propeller* prop1 = new sf::Propeller("Prop1", propeller1, 0.2, 0.1, 0.01, 10000, true);
    sf::Propeller* prop2 = new sf::Propeller("Prop2", propeller2, 0.2, 0.1, 0.01, 10000, true);
    sf::Propeller* prop3 = new sf::Propeller("Prop3", propeller3, 0.2, 0.1, 0.01, 10000, false);
    sf::Propeller* prop4 = new sf::Propeller("Prop4", propeller4, 0.2, 0.1, 0.01, 10000, false);
    
    sf::Box* arm1 = new sf::Box("Arm1", phy, sf::Vector3(1.0,0.01,0.01), sf::I4(), "Fiberglass", "white");
    sf::Box* arm2 = new sf::Box("Arm2", phy, sf::Vector3(0.01,1.0,0.01), sf::I4(), "Fiberglass", "white");
    sf::Compound* fuselage = new sf::Compound("Fuselage", phy, arm1, sf::I4());
    fuselage->AddExternalPart(arm2, sf::I4());
    
    sf::FeatherstoneRobot* quadCopter = new sf::FeatherstoneRobot("Quadcopter");
    quadCopter->DefineLinks(fuselage);
    quadCopter->BuildKinematicStructure();
    quadCopter->AddLinkActuator(prop1, "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.5,0.0,-0.02)));
    quadCopter->AddLinkActuator(prop2, "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(-0.5,0.0,-0.02)));
    quadCopter->AddLinkActuator(prop3, "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,0.5,-0.02)));
    quadCopter->AddLinkActuator(prop4, "Fuselage", sf::Transform(sf::Quaternion(0,M_PI_2,0), sf::Vector3(0.0,-0.5,-0.02)));
    AddRobot(quadCopter, sf::Transform(sf::IQ(), sf::Vector3(0,0,-0.02)));
    
    prop1->setSetpoint(0.5);
    prop2->setSetpoint(0.5);
    prop3->setSetpoint(-0.5);
    prop4->setSetpoint(-0.5);
}