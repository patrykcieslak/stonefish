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
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "FloatingTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <actuators/Thruster.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/Robot.h>
#include <entities/FeatherstoneEntity.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/Sample.h>

FloatingTestManager::FloatingTestManager(sf::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE, sf::FluidDynamicsType::GEOMETRY_BASED)
{
}

void FloatingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Fiberglass", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.5), 0.3);
    
    getMaterialManager()->SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    
    ///////LOOKS///////////
    int white = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.9f, 0.0f, 0.f);
    int propLook = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.3f, 0.f, 0.f, sf::GetDataPath() + "propeller_tex.png");

    ////////OBJECTS    
    //Create environment
    EnableOcean(0.0);
    getOcean()->SetupWaterProperties(0.2, 1.0);
    getAtmosphere()->SetupSunPosition(0.0, 60.0);
    
    sf::Polyhedron* hull = new sf::Polyhedron("Hull", sf::GetDataPath() + "boat_gra.obj", sf::Scalar(1), sf::I4(), sf::GetDataPath() + "boat.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), 
                                              sf::BodyPhysicsType::FLOATING_BODY, white, false, sf::Scalar(0.05), true);
    
    sf::Polyhedron* prop = new sf::Polyhedron("Propeller", sf::GetDataPath() + "propeller.obj", sf::Scalar(1), sf::I4(), getMaterialManager()->getMaterial("Fiberglass"), 
                                              sf::BodyPhysicsType::SUBMERGED_BODY, propLook, false);
    
    sf::Thruster* thrust = new sf::Thruster("Thruster", prop, 0.18, 0.48, 0.05, 10000.0, false);
    
    sf::Odometry* odom = new sf::Odometry("Odom");
    
    sf::Robot* boat = new sf::Robot("Boat");
    boat->DefineLinks(hull);
    boat->AddLinkActuator(thrust, "Hull", sf::Transform(sf::Quaternion(0,0,0), sf::Vector3(-0.8,0.02,0.3)));
    boat->AddLinkSensor(odom, "Hull", sf::I4());
    AddRobot(boat, sf::Transform(sf::IQ(), sf::Vector3(0,0,-1.0)));
    
    thrust->setSetpoint(0.2);
}

void FloatingTestManager::SimulationStepCompleted()
{
    /*sf::Robot* boat = getRobot("Boat");
    sf::Sample s = ((sf::Odometry*)boat->getSensor("Odom"))->getLastSample();
    printf("Velocity: %1.3lf, %1.3lf, %1.3lf\n", s.getValue(3), s.getValue(4), s.getValue(5));
    
    sf::FeatherstoneEntity* fe = (sf::FeatherstoneEntity*)getEntity("Boat_Dynamics");
    sf::Vector3 v = fe->getLink(0).solid->getLinearVelocityInLocalPoint(sf::Vector3(0,0,-0.5));
    printf("FE Velocity: %1.3lf, %1.3lf, %1.3lf\n", v.x(), v.y(), v.z());
    */
}