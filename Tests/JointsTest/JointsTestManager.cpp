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
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "JointsTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Cylinder.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLTrackball.h>
#include <joints/FixedJoint.h>
#include <joints/SphericalJoint.h>
#include <joints/RevoluteJoint.h>
#include <joints/PrismaticJoint.h>
#include <joints/CylindricalJoint.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>

JointsTestManager::JointsTestManager(sf::Scalar stepsPerSecond) 
  : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE, sf::FluidDynamicsType::GEOMETRY_BASED)
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
    getAtmosphere()->SetupSunPosition(0.0, 70.0);
    getTrackball()->MoveCenter(glm::vec3(1.f,3.f,0.f));
    
    sf::Plane* floor = new sf::Plane("Floor", 1000.f, "Steel", "grid");
    AddStaticEntity(floor, sf::I4());
    
    //----Fixed Joint----
    sf::Box* box = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(-2.0,0.0,-3.0)));
    
    sf::Sphere* sph = new sf::Sphere("Sph1", sf::Scalar(0.2), sf::I4(), "Steel", sf::BodyPhysicsType::SURFACE_BODY, "orange");
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(-2.0,-0.5,-3.0)));
    
    sf::FixedJoint* fixed = new sf::FixedJoint("Fix", box, sph);
    AddJoint(fixed);
    
    //----Revolute Joint----
    box = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0.5,0.0,-1.0)));
    
    sf::Box* box2 = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "orange");
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(0.5,0.2,-1.0)));
    
    sf::RevoluteJoint* revo = new sf::RevoluteJoint("Revolute", box, box2, sf::Vector3(0.5,0.1,-0.9), sf::Vector3(0,1,0), false);
    AddJoint(revo);
    
    sph = new sf::Sphere("Sph2", sf::Scalar(0.2), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "green");
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-3.0)));
    
    revo = new sf::RevoluteJoint("RevoluteFix", sph, sf::Vector3(0.0,1.0,-3.0), sf::Vector3(1.0,0.0,0.0));
    AddJoint(revo);
    
    //----Spherical Joint----
    sph = new sf::Sphere("Sph3", sf::Scalar(0.2), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "green");
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.0, -1.0)));
    sf::Sphere* sph2 = new sf::Sphere("Sph4", sf::Scalar(0.15), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "orange");
    AddSolidEntity(sph2, sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.2, -0.4)));
    
    sf::SphericalJoint* spher = new sf::SphericalJoint("Spherical", sph, sph2, sf::Vector3(0.0, -2.0, -0.6));
    AddJoint(spher);
    
    //----Prismatic Joint----
    box = new sf::Box("Box4", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(1.0,0.0,-0.051)));
    
    box2 = new sf::Box("Box5", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "orange");
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(1.0,0.0,-0.5)));
    
    sf::PrismaticJoint* trans = new sf::PrismaticJoint("Prismatic", box, box2, sf::Vector3(0.5,0,-1.0));
    AddJoint(trans);
    
    //----Cylindrical Joint----
    box = new sf::Box("Box6", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "green");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(-1.0,0.0,-0.051)));
    
    box2 = new sf::Box("Box7", sf::Vector3(0.1,0.1,0.1), sf::I4(), "Plastic", sf::BodyPhysicsType::SURFACE_BODY, "orange");
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(-1.0,0.0,-0.5)));
    
    sf::CylindricalJoint* cyli = new sf::CylindricalJoint("Cylindrical", box, box2, sf::Vector3(-1.0, 0.050, -0.25), sf::Vector3(0,0,1));
    AddJoint(cyli);
}
