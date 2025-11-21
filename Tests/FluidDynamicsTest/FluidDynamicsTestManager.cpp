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
//  Copyright(c) 2024-2025 Patryk Cieslak. All rights reserved.
//

#include "FluidDynamicsTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Compound.h>
#include <actuators/SimpleThruster.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/Sample.h>
#include <core/FeatherstoneRobot.h>
#include <graphics/OpenGLTrackball.h>
#include <utils/SystemUtil.hpp>
#include <utils/DebugUtil.hpp>
#include <utils/UnitSystem.h>
#include <core/NED.h>

FluidDynamicsTestManager::FluidDynamicsTestManager(sf::Scalar stepsPerSecond)
: SimulationManager(stepsPerSecond, sf::Solver::SI, sf::CollisionFilter::EXCLUSIVE)
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

    globalDamping = 0.1;
    setSolverParams(erp, stopErp, erp2, globalDamping, globalFriction, linSleep, angSleep);
    setFluidDynamicsPrescaler(1);

    ///////MATERIALS////////
    CreateMaterial("Light", sf::UnitSystem::Density(sf::CGS, sf::MKS, 0.9), 0.3);
    CreateMaterial("Neutral", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.0), 0.5);
    CreateMaterial("Heavy", sf::UnitSystem::Density(sf::CGS, sf::MKS, 2.0), 0.9);
    SetMaterialsInteraction("Light", "Light", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Heavy", 0.5, 0.2);
    SetMaterialsInteraction("Neutral", "Neutral", 0.5, 0.5);
    SetMaterialsInteraction("Light", "Neutral", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Neutral", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Light", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("Yellow", sf::Color::RGB(1.f, 0.9f, 0.1f), 0.3f, 0.f);
    CreateLook("Red", sf::Color::RGB(1.f, 0.1f, 0.1f), 0.3f, 0.f);
    
    ////////OBJECTS    
    EnableOcean(0.0);
    getOcean()->setWaterType(0.2);
    getAtmosphere()->SetSunPosition(0.0, 60.0);

    //Externals
    sf::PhysicsSettings phy;
    phy.mode = sf::PhysicsMode::SUBMERGED;
    phy.collisions = true;
    phy.buoyancy = true;

    //Sphere
    sf::Scalar sphereRadius = 0.5;
    sf::Sphere* sphere = new sf::Sphere("Sphere", phy, sphereRadius, sf::I4(), "Light", "Yellow");
    AddSolidEntity(sphere, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 0)));

    //Box
    sf::Vector3 boxDims(1.0, 0.5, 0.2);
    sf::Box* box = new sf::Box("Box", phy, boxDims, sf::I4(), "Light", "Yellow");
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(2, 0, 0)));

    //Compound
    //1st part - sphere
    sf::Sphere* comp1 = new sf::Sphere("Comp1", phy, sphereRadius, sf::I4(), "Light", "Yellow");
    //2nd part - box
    sf::Sphere* comp2 = new sf::Sphere("Comp2", phy, sphereRadius, sf::I4(), "Light", "Yellow");
    sf::Compound* compound = new sf::Compound("Compound", phy, comp1, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 0)));
    compound->AddExternalPart(comp2, sf::Transform(sf::IQ(), sf::Vector3(2.0, 0, 0)));
    AddSolidEntity(compound, sf::Transform(sf::Quaternion(0.0, 0.5, 0.0), sf::Vector3(6, 0, -5)));

    //Polyhedron
    sf::Polyhedron* hull = new sf::Polyhedron("Hull", phy, sf::GetDataPath() + "hull_hydro.obj", sf::Scalar(1), sf::I4(), "Light", "Yellow");
    AddSolidEntity(hull, sf::Transform(sf::Quaternion(0.0, 0.5, 0.0), sf::Vector3(8, 0, -5)));

    sf::Polyhedron* sphR1M = new sf::Polyhedron("SphereR1M", phy, sf::GetDataPath() + "sphere_R=1.obj", sf::Scalar(1), sf::I4(), "Light", "Yellow");
    AddSolidEntity(sphR1M, sf::Transform(sf::Quaternion(0.0, 0.5, 0.0), sf::Vector3(11, 0, -5)));

    //Featherstone robot
    sf::Box* link1 = new sf::Box("Link1", phy, boxDims, sf::I4(), "Light", "Yellow");
    sf::FeatherstoneRobot* robot = new sf::FeatherstoneRobot("Robot", false);
    robot->DefineLinks(link1);
    robot->BuildKinematicStructure();
    AddRobot(robot, sf::Transform(sf::IQ(), sf::Vector3(4, 0, 0)));

    // Drag testing
    // Single solid entities     
    sf::Box* hull1 = new sf::Box("Hull1", phy, boxDims, sf::I4(), "Neutral", "Yellow");
    AddSolidEntity(hull1, sf::Transform(sf::IQ(), sf::Vector3(-7, 0, 1)));

    sf::Box* hull2 = new sf::Box("Hull2", phy, boxDims, sf::I4(), "Neutral", "Yellow");
    AddSolidEntity(hull2, sf::Transform(sf::Quaternion(M_PI_2, 0, 0), sf::Vector3(-5, 2, 1)));

    std::shared_ptr<sf::Box> propeller = std::make_shared<sf::Box>("Propeller", phy, sf::Vector3(0.02,0.2,0.05), sf::I4(), "Neutral", "Red");
    
    sf::SimpleThruster* thruster1 = new sf::SimpleThruster("Thruster1", propeller, true, false);
    thruster1->AttachToSolid(hull1, sf::Transform(sf::IQ(), sf::Vector3(-0.6, 0, 0)));
    thruster1->setSetpoint(10.0, 0.0);
    AddActuator(thruster1);

    sf::SimpleThruster* thruster2 = new sf::SimpleThruster("Thruster2", propeller, true, false);
    thruster2->AttachToSolid(hull2, sf::Transform(sf::IQ(), sf::Vector3(-0.6, 0, 0)));
    thruster2->setSetpoint(10.0, 0.0);
    AddActuator(thruster2);

    // Compound solid entity
    for (int i = 0; i < 8; ++i)
    {
        sf::Box* part1 = new sf::Box("Part1_" + std::to_string(i), phy, boxDims, sf::I4(), "Neutral", "Yellow");
        sf::Box* part2 = new sf::Box("Part2_" + std::to_string(i), phy, boxDims, sf::I4(), "Neutral", "Yellow");
        sf::Compound* comp = new sf::Compound("Comp" + std::to_string(i), phy, part1, sf::Transform(sf::IQ(), sf::Vector3(0, 0.5, 0)));
        comp->AddExternalPart(part2, sf::Transform(sf::IQ(), sf::Vector3(0, -0.5, 0)));
        AddSolidEntity(comp, sf::Transform(sf::Quaternion(M_PI_4*i, 0.0, 0.0), sf::Vector3(0, 0, 2 + 2*i)));

        sf::Odometry* odometry = new sf::Odometry("Odometry" + std::to_string(i));
        odometry->AttachToSolid(comp, sf::Transform(sf::IQ(), sf::Vector3(0, 0, 0)));
        AddSensor(odometry);

        sf::SimpleThruster* thruster = new sf::SimpleThruster("ThrusterComp" + std::to_string(i), propeller, true, false);
        thruster->AttachToSolid(comp, sf::Transform(sf::IQ(), sf::Vector3(-0.6, 0, 0)));
        thruster->setSetpoint(20.0, 0.0);
        AddActuator(thruster);
    }
}

void FluidDynamicsTestManager::SimulationStepCompleted(sf::Scalar timeStep)
{
    std::cout << "----------------------------------------------------" << std::endl;

    sf::Scalar rho = getOcean()->getLiquid().density;

    //SPHERE
    sf::Sphere* sphere = (sf::Sphere*)getEntity("Sphere");
    sf::Scalar sphereRadius = 0.5;
    sf::Vector3 sphereInertia = sphere->getInertia();

    sf::Scalar expectedSphereVolume = 4.0 / 3.0 * M_PI * sphereRadius * sphereRadius * sphereRadius;
    sf::Scalar expectedSphereMass = sphere->getMaterial().density * expectedSphereVolume;
    sf::Scalar expectedSphereInertia = 2.0 / 5.0 * expectedSphereMass * sphereRadius * sphereRadius;
    sf::Scalar expectedSphereSubmergedVolume = expectedSphereVolume * sphere->getMaterial().density/rho;

    sf::testScalar("Sphere mass", sphere->getMass(), expectedSphereMass);
    sf::testScalar("Sphere volume", sphere->getVolume(), expectedSphereVolume);
    sf::testVector3("Sphere inertia", sphereInertia, sf::Vector3(expectedSphereInertia, expectedSphereInertia, expectedSphereInertia));
    sf::testScalar("Sphere submerged volume", sphere->getSubmergedVolume(), expectedSphereSubmergedVolume, 1e-3);
    
    //BOX
    sf::Box* box = (sf::Box*)getEntity("Box");
    sf::Vector3 boxDims(1.0, 0.5, 0.2);
    sf::Vector3 boxInertia = box->getInertia();

    sf::Scalar expectedBoxVolume = boxDims.getX() * boxDims.getY() * boxDims.getZ();
    sf::Scalar expectedBoxMass = box->getMaterial().density * expectedBoxVolume;
    sf::Vector3 expectedBoxInertia = 1.0 / 12.0 * expectedBoxMass * sf::Vector3(boxDims.getY() * boxDims.getY() + boxDims.getZ() * boxDims.getZ(),
        boxDims.getX() * boxDims.getX() + boxDims.getZ() * boxDims.getZ(), boxDims.getX() * boxDims.getX() + boxDims.getY() * boxDims.getY());
    sf::Scalar expectedBoxSubmergedVolume = expectedBoxVolume * box->getMaterial().density/rho;

    sf::testScalar("Box mass", box->getMass(), expectedBoxMass);
    sf::testScalar("Box volume", box->getVolume(), expectedBoxVolume);
    sf::testVector3("Box inertia", boxInertia, expectedBoxInertia);
    sf::testScalar("Box submerged volume", box->getSubmergedVolume(), expectedBoxSubmergedVolume, 1e-3);

    //POLYHEDRON
    sf::Polyhedron* hull = (sf::Polyhedron*)getEntity("Hull");
    sf::Vector3 hullInertia = hull->getInertia();
    sf::Transform cgTransform = hull->getCG2OTransform().inverse();
    sf::Scalar expectedHullVolume = 0.096699;
    sf::Scalar expectedHullMass = hull->getMaterial().density * expectedHullVolume;
    sf::Vector3 expectedHullInertia = hull->getMaterial().density * sf::Vector3(0.001080, 0.014772, 0.014772);
    sf::Scalar expectedHullSubmergedVolume = expectedHullVolume * hull->getMaterial().density/rho;
    
    sf::testScalar("Hull mass", hull->getMass(), expectedHullMass);
    sf::testScalar("Hull volume", hull->getVolume(), expectedHullVolume);
    sf::testVector3("Hull inertia", hullInertia, expectedHullInertia);
    sf::testScalar("Hull submerged volume", hull->getSubmergedVolume(), expectedHullSubmergedVolume);
    
    //POLYHEDRON 2
    sf::Polyhedron* sphR1M = (sf::Polyhedron*)getEntity("SphereR1M");
    sf::Vector3 sphR1MInertia = sphR1M->getInertia();
    sf::Transform sphR1MCGTransform = sphR1M->getCG2OTransform().inverse();

    sf::Scalar expectedSphR1MVolume = 4.0 / 3.0 * M_PI;
    sf::Scalar expectedSphR1MMass = sphR1M->getMaterial().density * expectedSphR1MVolume;
    sf::Vector3 expectedSphR1MInertia = 2.0 / 5.0 * expectedSphR1MMass * sf::Vector3(1, 1, 1);
    sf::Scalar expectedSphR1MSubmergedVolume = expectedSphR1MVolume * sphR1M->getMaterial().density/rho;

    sf::testScalar("SphereR1M mass", sphR1M->getMass(), expectedSphR1MMass, 1e-2);
    sf::testScalar("SphereR1M volume", sphR1M->getVolume(), expectedSphR1MVolume, 1e-2);
    sf::testVector3("SphereR1M inertia", sphR1MInertia, expectedSphR1MInertia, 1e-2);
    sf::testScalar("SphereR1M submerged volume", sphR1M->getSubmergedVolume(), expectedSphR1MSubmergedVolume, 1e-2);

    // DRAG TESTING
    auto v1 = static_cast<sf::SolidEntity*>(getEntity("Hull1"))->getLinearVelocity();
    auto v2 = static_cast<sf::SolidEntity*>(getEntity("Hull2"))->getLinearVelocity();

    sf::testScalar("Solid body drag", v1.length(), v2.length(), 0.01);

    auto odometry = getSensor("Odometry0");
    auto s = static_cast<sf::Odometry*>(odometry)->getLastSample();
    sf::Scalar velX = s.getValue(3);

    for (int i = 1; i < 8; ++i)
    {
        auto odometry = getSensor("Odometry" + std::to_string(i));
        auto s = static_cast<sf::Odometry*>(odometry)->getLastSample();   
        sf::testScalar("Comp" + std::to_string(i) + " velocity", s.getValue(3), velX, 0.01);
    }

    std::cout << std::endl;
}