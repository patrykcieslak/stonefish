//
//  JointsTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "JointsTestManager.h"

#include "JointsTestApp.h"
#include "Plane.h"
#include "Box.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "OpenGLPointLight.h"
#include "OpenGLTrackball.h"
#include "FixedJoint.h"
#include "SphericalJoint.h"
#include "RevoluteJoint.h"
#include "PrismaticJoint.h"
#include "CylindricalJoint.h"
#include "GearJoint.h"
#include "BeltJoint.h"
#include "DCMotor.h"

JointsTestManager::JointsTestManager(btScalar stepsPerSecond) : SimulationManager(SimulationType::TERRESTIAL, UnitSystems::MKS, stepsPerSecond, SolverType::SI)
{
}

void JointsTestManager::BuildScenario()
{
    //--------------------Using MMSK unit system--------------------
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MKS, 7.8), 0.5);
    getMaterialManager()->CreateMaterial("Plastic", UnitSystem::Density(CGS, MKS, 1.5), 0.2);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Plastic", 0.8, 0.2);
    getMaterialManager()->SetMaterialsInteraction("Steel", "Steel", 0.5, 0.1);
    getMaterialManager()->SetMaterialsInteraction("Plastic", "Plastic", 0.5, 0.2);
    
    ///////LOOKS///////////
    int orange = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(1.0f,0.6f,0.3f), 0.3f, 0.1f);
    int green = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(0.5f,1.0f,0.4f), 0.5f, 0.9f);
    
    ////////OBJECTS
    //----Fixed Joint----
    Box* box = new Box("Box", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0.0,0.0,1.0)));
    Sphere* sph = new Sphere("Sph1", 0.2, getMaterialManager()->getMaterial("Steel"), orange);
    AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0.0,-0.5,1.0)));
    
    FixedJoint* fixed = new FixedJoint("Fix", box, sph);
    AddJoint(fixed);
    
    //----Revolute Joint----
    box = new Box("Box", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0.5,0.0,1.0)));
    
    Box* box2 = new Box("Box", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, btTransform(btQuaternion::getIdentity(), btVector3(0.5,0.2,1.0)));
    
    RevoluteJoint* revo = new RevoluteJoint("Revolute", box, box2, btVector3(0.5,0.1,0.9), btVector3(0,1,0), false);
    AddJoint(revo);
    
    //----Spherical Joint----
    sph = new Sphere("Sph2", 0.2, getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0.0, -2.0,1.0)));
    Sphere* sph2 = new Sphere("Sph3", 0.15, getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(sph2, btTransform(btQuaternion::getIdentity(), btVector3(0.0,-2.2,0.4)));
    
    SphericalJoint* spher = new SphericalJoint("Spherical", sph, sph2, btVector3(0.0, -2.0, 0.6));
    AddJoint(spher);
    
    //----Prismatic Joint----
    box = new Box("Box4", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(1.0,0.0,0.051)));
    
    box2 = new Box("Box5", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, btTransform(btQuaternion::getIdentity(), btVector3(1.0,0.0,0.5)));
    
    PrismaticJoint* trans = new PrismaticJoint("Prismatic", box, box2, btVector3(0.5,0,1));
    AddJoint(trans);
    
    //----Cylindrical Joint----
    box = new Box("Box6", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(-1.0,0.0,0.051)));
    
    box2 = new Box("Box7", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, btTransform(btQuaternion::getIdentity(), btVector3(-1.0,0.0,0.5)));
    
    CylindricalJoint* cyli = new CylindricalJoint("Cylindrical", box, box2, btVector3(-1.0, 0.050, 0.25), btVector3(0,0,1));
    AddJoint(cyli);
    
    //----Gear Joint----
    box = new Box("Box", btVector3(1.0,1.0,1.0), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(2.0,2.0,1.0)));
    
    Cylinder* cyl = new Cylinder("Cyl1", 0.2, 0.020, getMaterialManager()->getMaterial("Steel"), green);
    AddSolidEntity(cyl, btTransform(btQuaternion::getIdentity(), btVector3(2.0, 2.0, 2.0)));
    
    Cylinder* cyl2 = new Cylinder("Cyl2", 0.1, 0.020, getMaterialManager()->getMaterial("Steel"), orange);
    AddSolidEntity(cyl2, btTransform(btQuaternion(0,0,M_PI_4), btVector3(2.0, 1.93, 2.56)));
    
    revo = new RevoluteJoint("GearRevolute1", box, cyl, btVector3(2.0, 2.0, 2.0), btVector3(0,1,0), false);
    AddJoint(revo);
    
    revo = new RevoluteJoint("GearRevolute2", box, cyl2, btVector3(2.0, 1.93, 2.56), btVector3(0,1,1), false);
    AddJoint(revo);
    
    GearJoint* gear = new GearJoint("Gear", cyl2, cyl, btVector3(0,1,1), btVector3(0,1,0), 2.0);
    gear->setRenderable(true);
    AddJoint(gear);
}