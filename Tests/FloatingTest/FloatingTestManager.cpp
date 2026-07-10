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
//  FloatingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/05/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "FloatingTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <actuators/Thruster.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/FeatherstoneRobot.h>
#include <entities/FeatherstoneEntity.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/Sample.h>

FloatingTestManager::FloatingTestManager(sf::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
{
}

void FloatingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Fiberglass", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.5), 0.3);
    SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("white", sf::Color::Gray(1.f), 0.9f, 0.0f, 0.f);
    CreateLook("propeller", sf::Color::Gray(1.f), 0.3f, 0.f, 0.f, sf::GetDataPath() + "propeller_tex.png");

    ////////OBJECTS    
    //Create environment
    EnableOcean(0.0);
    getOcean()->setWaterType(0.2);
    getAtmosphere()->SetSunPosition(0.0, 60.0);
    
    //Hull
    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::FLOATING;
    phy.collisions = true;
    phy.buoyancy = true;
    std::unique_ptr<sf::Polyhedron> hull = std::make_unique<sf::Polyhedron>("Hull", phy, sf::GetDataPath() + "boat_gra.obj", sf::Scalar(1), sf::I4(), 
        sf::GetDataPath() + "boat.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "white", sf::Scalar(0.09));
    hull->ScalePhysicalPropertiesToArbitraryMass(150.0);

    //Propeller
    phy.mode = sf::PhysicsMode::SUBMERGED;
    phy.buoyancy = false;

    std::unique_ptr<sf::Polyhedron> propeller = std::make_unique<sf::Polyhedron>("Propeller", phy, sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), "Fiberglass", "propeller");
    std::unique_ptr<sf::MechanicalPI> rotorDynamics =  std::make_unique<sf::MechanicalPI>(1.0, 10.0, 5.0, 5.0);
    std::unique_ptr<sf::FDThrust> thrustModel = std::make_unique<sf::FDThrust>(0.18, 0.48, 0.48, 0.05, true, getOcean()->getLiquid().density);
    std::unique_ptr<sf::Thruster> thrust = std::make_unique<sf::Thruster>("Thruster", std::move(propeller), std::move(rotorDynamics), std::move(thrustModel), 
        0.18, true, 105.0, false, true);

    //Sensors   
    std::unique_ptr<sf::Odometry> odom = std::make_unique<sf::Odometry>("Odom");
    
    //Boat
    std::unique_ptr<sf::FeatherstoneRobot> boat = std::make_unique<sf::FeatherstoneRobot>("Boat");
    boat->DefineLinks(std::move(hull));
    boat->BuildKinematicStructure();
    sf::LinkActuator* th = boat->AddLinkActuator(std::move(thrust), "Hull", sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(-0.8,0.02,0.3)));
    boat->AddLinkSensor(std::move(odom), "Hull", sf::I4());
    AddRobot(std::move(boat), sf::Transform(sf::IQ(), sf::Vector3(0,0,0)));
    
    static_cast<sf::Thruster*>(th)->setSetpoint(1.0);
}