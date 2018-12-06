//
//  JointsTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "JointsTestManager.h"

#include "JointsTestApp.h"
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
#include <joints/GearJoint.h>
#include <joints/BeltJoint.h>
#include <actuators/DCMotor.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>

JointsTestManager::JointsTestManager(sf::Scalar stepsPerSecond) : sf::SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI)
{
}

void JointsTestManager::BuildScenario()
{
    //--------------------Using MMSK unit system--------------------
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", sf::UnitSystem::Density(sf::UnitSystems::CGS, sf::UnitSystems::MKS, 7.8), 0.5);
    getMaterialManager()->CreateMaterial("Plastic", sf::UnitSystem::Density(sf::UnitSystems::CGS, sf::UnitSystems::MKS, 1.5), 0.2);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Plastic", 0.8, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.5, 0.1);
    getMaterialManager()->SetMaterialsInteraction("Plastic", "Plastic", 0.5, 0.2);
    
    ///////LOOKS///////////
    int grid = CreateLook(sf::Color(1.f, 1.f, 1.f), 0.5f, 0.0f, 0.0f, sf::GetShaderPath() + "grid.png");
    int orange = CreateLook(sf::Color(1.0f,0.6f,0.3f), 0.3f);
    int green = CreateLook(sf::Color(0.5f,1.0f,0.4f), 0.5f);
    
    ////////OBJECTS
    sf::Plane* floor = new sf::Plane("Floor", 1000.f, getMaterialManager()->getMaterial("Steel"), grid);
    AddStaticEntity(floor, sf::I4());
    
    //----Fixed Joint----
    sf::Box* box = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0.0,0.0,-1.0)));
    
    sf::Sphere* sph = new sf::Sphere("Sph1", sf::Scalar(0.2), sf::I4(), getMaterialManager()->getMaterial("Steel"), orange);
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0,-0.5,-1.0)));
    
    sf::FixedJoint* fixed = new sf::FixedJoint("Fix", box, sph);
    AddJoint(fixed);
    
    
    //----Revolute Joint----
    box = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, sf::Transform(sf::IQ(), sf::Vector3(0.5,0.0,-1.0)));
    
    sf::Box* box2 = new sf::Box("Box", sf::Vector3(0.1,0.1,0.1), sf::I4(), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, sf::Transform(sf::IQ(), sf::Vector3(0.5,0.2,-1.0)));
    
    sf::RevoluteJoint* revo = new sf::RevoluteJoint("Revolute", box, box2, sf::Vector3(0.5,0.1,-0.9), sf::Vector3(0,1,0), false);
    AddJoint(revo);
    
    //----Spherical Joint----
    sph = new sf::Sphere("Sph2", sf::Scalar(0.2), sf::I4(), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(sph, sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.0, -1.0)));
    sf::Sphere* sph2 = new sf::Sphere("Sph3", sf::Scalar(0.15), sf::I4(), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(sph2, sf::Transform(sf::IQ(), sf::Vector3(0.0, -2.2, -0.4)));
    
    sf::SphericalJoint* spher = new sf::SphericalJoint("Spherical", sph, sph2, sf::Vector3(0.0, -2.0, -0.6));
    AddJoint(spher);
    
    /*
    //----Prismatic Joint----
    box = new Box("Box4", Vector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, Transform(Quaternion::getIdentity(), Vector3(1.0,0.0,0.051)));
    
    box2 = new Box("Box5", Vector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, Transform(Quaternion::getIdentity(), Vector3(1.0,0.0,0.5)));
    
    PrismaticJoint* trans = new PrismaticJoint("Prismatic", box, box2, Vector3(0.5,0,1));
    AddJoint(trans);
    
    //----Cylindrical Joint----
    box = new Box("Box6", Vector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, Transform(Quaternion::getIdentity(), Vector3(-1.0,0.0,0.051)));
    
    box2 = new Box("Box7", Vector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, Transform(Quaternion::getIdentity(), Vector3(-1.0,0.0,0.5)));
    
    CylindricalJoint* cyli = new CylindricalJoint("Cylindrical", box, box2, Vector3(-1.0, 0.050, 0.25), Vector3(0,0,1));
    AddJoint(cyli);
    
    //----Gear Joint----
    box = new Box("Box", Vector3(1.0,1.0,1.0), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, Transform(Quaternion::getIdentity(), Vector3(2.0,2.0,1.0)));
    
    Cylinder* cyl = new Cylinder("Cyl1", 0.2, 0.020, getMaterialManager()->getMaterial("Steel"), green);
    AddSolidEntity(cyl, Transform(Quaternion::getIdentity(), Vector3(2.0, 2.0, 2.0)));
    
    Cylinder* cyl2 = new Cylinder("Cyl2", 0.1, 0.020, getMaterialManager()->getMaterial("Steel"), orange);
    AddSolidEntity(cyl2, Transform(Quaternion(0,0,M_PI_4), Vector3(2.0, 1.93, 2.56)));
    
    revo = new RevoluteJoint("GearRevolute1", box, cyl, Vector3(2.0, 2.0, 2.0), Vector3(0,1,0), false);
    AddJoint(revo);
    
    revo = new RevoluteJoint("GearRevolute2", box, cyl2, Vector3(2.0, 1.93, 2.56), Vector3(0,1,1), false);
    AddJoint(revo);
    
    GearJoint* gear = new GearJoint("Gear", cyl2, cyl, Vector3(0,1,1), Vector3(0,1,0), 2.0);
    gear->setRenderable(true);
    AddJoint(gear);*/
}
