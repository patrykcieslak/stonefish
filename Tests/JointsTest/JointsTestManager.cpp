//
//  JointsTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "JointsTestManager.h"

#include "JointsTestApp.h"
#include "PlaneEntity.h"
#include "BoxEntity.h"
#include "SphereEntity.h"
#include "CylinderEntity.h"
#include "OpenGLSpotLight.h"
#include "OpenGLTrackball.h"
#include "FixedJoint.h"
#include "SphericalJoint.h"
#include "RevoluteJoint.h"
#include "PrismaticJoint.h"
#include "CylindricalJoint.h"
#include "GearJoint.h"
#include "BeltJoint.h"

JointsTestManager::JointsTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond)
{
}

void JointsTestManager::BuildScenario()
{
    SimulationManager::BuildScenario();
    
    //--------------------Using MMSK unit system--------------------
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Steel", UnitSystem::Density(CGS, MMKS, 7.8), 0.8, 0.5, 0.05);
    getMaterialManager()->CreateMaterial("Plastic", UnitSystem::Density(CGS, MMKS, 1.5), 0.2, 0.5, 0.05);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.8f);
    Look orange = CreateGlossyLook(1.f, 0.5f, 0.2f, 0.5f, 0.f);
    Look green = CreateMatteLook(0.2f, 1.f, 0.3f, 0.5f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 1000000.f, getMaterialManager()->getMaterial("Steel"), grey, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0.f)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    //----Fixed Joint----
    BoxEntity* box = new BoxEntity("Box1", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0.0,0.0,1000.0)));
    SphereEntity* sph = new SphereEntity("Sph1", 200.0, getMaterialManager()->getMaterial("Steel"), orange);
    AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0.0,-500.0,1000.0)));
    
    FixedJoint* fixed = new FixedJoint("Fix", box, sph);
    fixed->setRenderable(true);
    AddJoint(fixed);
    
    //----Revolute Joint----
    box = new BoxEntity("Box2", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), green);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(500.0,0.0,1000.0)));
    
    BoxEntity* box2 = new BoxEntity("Box3", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, btTransform(btQuaternion::getIdentity(), btVector3(500.0,200.0,1000.0)));
    
    RevoluteJoint* revo = new RevoluteJoint("Revolute", box, box2, btVector3(500.0,100.0,100.0), btVector3(0,1,0), false);
    revo->setRenderable(true);
    AddJoint(revo);
    
    //----Spherical Joint----
    sph = new SphereEntity("Sph2", 200.0, getMaterialManager()->getMaterial("Plastic"), green, true);
    AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0.0, -2000.0,1000.0)));
    SphereEntity* sph2 = new SphereEntity("Sph3", 150.0, getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(sph2, btTransform(btQuaternion::getIdentity(), btVector3(0.0,-2200.0,400.0)));
    
    SphericalJoint* spher = new SphericalJoint("Spherical", sph, sph2, btVector3(0.0, -2000.0, 600.0));
    spher->setRenderable(true);
    AddJoint(spher);
    
    //----Prismatic Joint----
    box = new BoxEntity("Box4", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), green, true);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(1000.0,0.0,51.0)));
    
    box2 = new BoxEntity("Box5", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, btTransform(btQuaternion::getIdentity(), btVector3(1000.0,0.0,500.0)));
    
    PrismaticJoint* trans = new PrismaticJoint("Prismatic", box, box2, btVector3(0.5,0,1));
    trans->setRenderable(true);
    AddJoint(trans);
    
    //----Cylindrical Joint----
    box = new BoxEntity("Box6", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), green, true);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(-1000.0,0.0,51.0)));
    
    box2 = new BoxEntity("Box7", btVector3(100.0,100.0,100.0), getMaterialManager()->getMaterial("Plastic"), orange);
    AddSolidEntity(box2, btTransform(btQuaternion::getIdentity(), btVector3(-1000.0,0.0,500.0)));
    
    CylindricalJoint* cyli = new CylindricalJoint("Cylindrical", box, box2, btVector3(0,0,1));
    cyli->setRenderable(true);
    AddJoint(cyli);
    
    //----Gear Joint----
    CylinderEntity* cyl = new CylinderEntity("Cyl1", 200.0, 20.0, getMaterialManager()->getMaterial("Steel"), green);
    AddSolidEntity(cyl, btTransform(btQuaternion::getIdentity(), btVector3(0.0, 1000.0, 300.0)));
    
    CylinderEntity* cyl2 = new CylinderEntity("Cyl2", 100.0, 20.0, getMaterialManager()->getMaterial("Steel"), green);
    AddSolidEntity(cyl2, btTransform(btQuaternion::getIdentity(), btVector3(0.0, 1000.0, 600.0)));
    
    revo = new RevoluteJoint("GearRevolute1", box, cyl, btVector3(0.0, 1000.0, 300.0), btVector3(0,1,0), false);
    AddJoint(revo);
    
    revo = new RevoluteJoint("GearRevolute2", box, cyl2, btVector3(0.0, 1000.0, 600.0), btVector3(0,1,0), false);
    ((btHingeConstraint*)revo->getConstraint())->enableAngularMotor(true, 0.1, 100.0);
    AddJoint(revo);
    
    GearJoint* gear = new GearJoint("Gear", cyl2, cyl, btVector3(0,1,0), btVector3(0,1,0), 2.0);
    gear->setRenderable(true);
    AddJoint(gear);
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 200.f, 500.f), 2000.f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 0, 60.f);
    trackb->Rotate(btQuaternion(M_PI+M_PI_4, 0.0, 0.0));
    trackb->Activate();
    AddView(trackb);
}