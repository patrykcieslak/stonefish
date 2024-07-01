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
//  FluidDynamicsTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 01/07/2024.
//  Copyright(c) 2024 Patryk Cieslak. All rights reserved.
//

#include "FluidDynamicsTestManager.h"

#include "FluidDynamicsTestApp.h"
#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Compound.h>
#include <core/FeatherstoneRobot.h>
#include <graphics/OpenGLTrackball.h>
#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>
#include <core/NED.h>

FluidDynamicsTestManager::FluidDynamicsTestManager(sf::Scalar stepsPerSecond)
: SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE)
{
}

void FluidDynamicsTestManager::BuildScenario()
{
    ///////SOLVER///////////
    sf::Scalar erp, stopErp;
    getJointErp(erp, stopErp);
    sf::Scalar erp2 = getDynamicsWorld()->getSolverInfo().m_erp2;
    sf::Scalar globalDamping = getDynamicsWorld()->getSolverInfo().m_damping;
    sf::Scalar globalFriction = getDynamicsWorld()->getSolverInfo().m_friction;
    sf::Scalar linSleep, angSleep;
    getSleepingThresholds(linSleep, angSleep);

    globalDamping = 0.9;
    setSolverParams(erp, stopErp, erp2, globalDamping, globalFriction, linSleep, angSleep);
    setFluidDynamicsPrescaler(1);

    ///////MATERIALS////////
    CreateMaterial("Light", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.9), 0.3);
    CreateMaterial("Heavy", sf::UnitSystem::Density(sf::CGS, sf::MKS, 2.0), 0.9);
    SetMaterialsInteraction("Light", "Light", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Heavy", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Light", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("Yellow", sf::Color::RGB(1.f, 0.9f, 0.f), 0.3f, 0.f);
    
    ////////OBJECTS    
    EnableOcean(0.0);
    getOcean()->setWaterType(0.2);
    getAtmosphere()->SetupSunPosition(0.0, 60.0);

    //Externals
    sf::BodyPhysicsSettings phy;
    phy.mode = sf::BodyPhysicsMode::SUBMERGED;
    phy.collisions = true;
    phy.buoyancy = true;

    //Sphere
    sf::Scalar sphereRadius = 0.5;
    sf::Sphere* sphere = new sf::Sphere("Sphere", phy, sphereRadius, sf::I4(), "Light", "Yellow");
    AddSolidEntity(sphere, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 0)));

    sf::Scalar sphereVolume = sphere->getVolume();
    sf::Scalar sphereMass = sphere->getMass();
    sf::Vector3 sphereInertia = sphere->getInertia();
    
    sf::Scalar expectedSphereVolume = 4.0 / 3.0 * M_PI * sphereRadius * sphereRadius * sphereRadius;
    sf::Scalar expectedSphereMass = sphere->getMaterial().density * expectedSphereVolume;
    sf::Scalar expectedSphereInertia = 2.0 / 5.0 * expectedSphereMass * sphereRadius * sphereRadius;

    printf("Sphere mass: %1.3lf (expected %1.3lf)\n", sphereMass, expectedSphereMass);
    printf("Sphere volume: %1.3lf (expected %1.3lf)\n", sphereVolume, expectedSphereVolume);
    printf("Sphere inertia: [%1.3lf, %1.3lf, %1.3lf] (expected [%1.3lf, %1.3lf, %1.3lf])\n", 
        sphereInertia.getX(), sphereInertia.getY(), sphereInertia.getZ(), expectedSphereInertia, expectedSphereInertia, expectedSphereInertia);

    //Box
    sf::Vector3 boxDims(0.1, 0.2, 0.5);
    sf::Box* box = new sf::Box("Box", phy, boxDims, sf::I4(), "Light", "Yellow");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(2, 0, 0)));

    sf::Scalar boxVolume = box->getVolume();
    sf::Scalar boxMass = box->getMass();
    sf::Vector3 boxInertia = box->getInertia();

    sf::Scalar expectedBoxVolume = boxDims.getX() * boxDims.getY() * boxDims.getZ();
    sf::Scalar expectedBoxMass = box->getMaterial().density * expectedBoxVolume;
    sf::Vector3 expectedBoxInertia = 1.0 / 12.0 * expectedBoxMass * sf::Vector3(boxDims.getY() * boxDims.getY() + boxDims.getZ() * boxDims.getZ(),
        boxDims.getX() * boxDims.getX() + boxDims.getZ() * boxDims.getZ(), boxDims.getX() * boxDims.getX() + boxDims.getY() * boxDims.getY());

    printf("Box mass: %1.3lf (expected %1.3lf)\n", boxMass, expectedBoxMass);
    printf("Box volume: %1.3lf (expected %1.3lf)\n", boxVolume, expectedBoxVolume);
    printf("Box inertia: [%1.3lf, %1.3lf, %1.3lf] (expected [%1.3lf, %1.3lf, %1.3lf])\n", 
        boxInertia.getX(), boxInertia.getY(), boxInertia.getZ(), expectedBoxInertia.getX(), expectedBoxInertia.getY(), expectedBoxInertia.getZ());

    //Compound
    //1st part - sphere
    sf::Sphere* comp1 = new sf::Sphere("Comp1", phy, sphereRadius, sf::I4(), "Light", "Yellow");
    //2nd part - box
    sf::Sphere* comp2 = new sf::Sphere("Comp2", phy, sphereRadius, sf::I4(), "Light", "Yellow");

    sf::Compound* compound = new sf::Compound("Compound", phy, comp1, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 0)));
    compound->AddExternalPart(comp2, sf::Transform(sf::IQ(), sf::Vector3(2.0, 0, 0)));
    AddSolidEntity(compound, sf::Transform(sf::Quaternion(0.0, 0.5, 0.0), sf::Vector3(6, 0, -5)));

    //Featherstone robot
    sf::Box* link1 = new sf::Box("Link1", phy, boxDims, sf::I4(), "Light", "Yellow");
    sf::FeatherstoneRobot* robot = new sf::FeatherstoneRobot("Robot", false);
    robot->DefineLinks(link1);
    robot->BuildKinematicStructure();
    AddRobot(robot, sf::Transform(sf::IQ(), sf::Vector3(4, 0, 0)));
} 

void FluidDynamicsTestManager::SimulationStepCompleted(sf::Scalar timeStep)
{
    sf::Sphere* sphere = (sf::Sphere*)getEntity("Sphere");
    sf::Scalar sphereRadius = 0.5;
    sf::Scalar sphereSubmergedVolume = sphere->getSubmergedVolume();
    sf::Scalar expectedSphereSubmergedVolume = 4.0 / 3.0 * M_PI * sphereRadius * sphereRadius * sphereRadius * sphere->getMaterial().density;
    printf("Sphere submerged volume: %1.3lf (expected %1.3lf)\n", sphereSubmergedVolume, expectedSphereSubmergedVolume);
}