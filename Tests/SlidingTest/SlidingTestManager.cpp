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
//  SlidingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestManager.h"

#include "SlidingTestApp.h"
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Box.h>
#include <actuators/Light.h>
#include <sensors/scalar/Trajectory.h>
#include <utils/SystemUtil.hpp>

SlidingTestManager::SlidingTestManager(sf::Scalar stepsPerSecond) 
  : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE, sf::FluidDynamicsType::GEOMETRY_BASED)
{
}

void SlidingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Ground", 1000.0, 1.0);
    getMaterialManager()->CreateMaterial("Steel", 1000.0, 0.0);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Ground", 0.0, 0.0);
    getMaterialManager()->SetMaterialsInteraction("Ground", "Steel", 0.25, 0.1);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.25, 0.2);
    
    ///////LOOKS///////////
    int grid = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.f, 0.1f, 0.f, sf::GetShaderPath() + "grid.png");
    int green = CreateLook(sf::Color::RGB(0.3f, 1.0f, 0.2f), 0.2f, 0.f);
    
    ////////OBJECTS
    sf::Scalar angle = M_PI/180.0 * 14.9;
    
    sf::Plane* floor = new sf::Plane("Floor", sf::Scalar(10000), getMaterialManager()->getMaterial("Ground"), grid);
    AddStaticEntity(floor, sf::I4());
  
    sf::Obstacle* ramp = new sf::Obstacle("Ramp", sf::Vector3(10,2,0.1), getMaterialManager()->getMaterial("Ground"), grid, 2);
    AddStaticEntity(ramp, sf::Transform(sf::Quaternion(0, angle, 0), sf::Vector3(0,0,-1.0)));
  
    sf::Box* box = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), getMaterialManager()->getMaterial("Steel"), sf::BodyPhysicsType::SURFACE_BODY, green);
    AddSolidEntity(box, sf::Transform(sf::Quaternion(0, angle, 0), sf::Vector3(2.5, 0, -2.0)));
    
    sf::Trajectory* traj = new sf::Trajectory("Trajectory", -1, 1000);
    traj->AttachToSolid(box, sf::I4());
    traj->setRenderable(true);
    AddSensor(traj);
    
    //////CAMERA & LIGHT//////
    sf::Light* omni = new sf::Light("Omni", sf::Color::BlackBody(4000), 1000000);
    omni->AttachToSolid(box, sf::Transform(sf::IQ(), sf::Vector3(0,0,-1)));
    AddActuator(omni);
}
