//
//  RollingTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RollingTestManager.h"

#include "RollingTestApp.h"
#include "PlaneEntity.h"
#include "BoxEntity.h"
#include "TorusEntity.h"
#include "CylinderEntity.h"
#include "OpenGLOmniLight.h"
#include "OpenGLTrackball.h"
#include "FixedJoint.h"
#include "RevoluteJoint.h"
#include "FakeIMU.h"
#include "SystemUtil.h"
#include "DCMotor.h"
#include "Trajectory.h"

RollingTestManager::RollingTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond, DANTZIG, INCLUSIVE)
{
}

void RollingTestManager::BuildScenario()
{
    //--------------------Using MMSK unit system--------------------
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MMKS, 4.0), 0.5);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MMKS, 2.0), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.75, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.8);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.8, 0.7);
    
    ///////LOOKS///////////
    char path[1024];
    GetCWD(path, 1024);
    GetDataPath(path, 1024-32);
    strcat(path, "grid.png");
    
    Look grid = CreateMatteLook(1.f, 1.f, 1.f, 0.0f, path);
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.0f);
    Look shiny = CreateMatteLook(0.2f, 0.2f, 0.2f, 0.5f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 2000000.f, getMaterialManager()->getMaterial("Concrete"), grid, btTransform(btQuaternion(0,0,0), btVector3(0,0,-1000.f)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    TorusEntity* torus = new TorusEntity("Wheel", 200.f, 20.f, getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(torus, btTransform(btQuaternion(0.f,0.f,0.0f), btVector3(0,0,300.f)));
    
    CylinderEntity* cyl = new CylinderEntity("Cart", 170.f, 20.f, getMaterialManager()->getMaterial("Concrete"), grey);
    AddSolidEntity(cyl, btTransform(btQuaternion(0,0,0), btVector3(0,0, 300.f)));
    cyl->SetArbitraryPhysicalProperties(1.5f, btVector3(1500,1500,1500), btTransform(btQuaternion::getIdentity(), btVector3(0,0.0f,-50.f)));
    
    BoxEntity* box = new BoxEntity("Lever", btVector3(20.f,20.f,150.f), getMaterialManager()->getMaterial("Concrete"), grey);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0.f, 0.f, 300.f-150.f/2.f)));
    
    RevoluteJoint* revo = new RevoluteJoint("CartWheel", cyl, torus, btVector3(0,0,300.f), btVector3(0,1.f,0), false);
    AddJoint(revo);
    revo->setDamping(0.01, 0.01);
    
    DCMotor* motor = new DCMotor("DCXWheel", revo, 0.212f, 0.0774e-3, 1.f/408.f, 23.4e-3, 0.0000055f);
    AddActuator(motor);
    motor->setVoltage(-1.f);
    
    revo = new RevoluteJoint("CartLever", box, cyl, btVector3(0.f,0.f,300.f), btVector3(1.f,0,0), false);
    AddJoint(revo);
    revo->setDamping(0.0, 0.01);
    
    motor = new DCMotor("DCXLever", revo, 0.212f, 0.0774e-3, 1.f/408.f, 23.4e-3, 0.0000055f);
    AddActuator(motor);
    //motor->setVoltage(0.0001f);
    
    
    Contact* c = AddContact(torus, floor, 10000);
    c->setRenderType(CONTACT_RENDER_PATH);
    
    FakeIMU* imu = new FakeIMU("IMU", cyl, btTransform::getIdentity(), 1000);
    AddSensor(imu);
    
    Trajectory* traj = new Trajectory("Trajectory", torus, btVector3(0,0,100), 1000);
    traj->setRenderable(true);
    AddSensor(traj);
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 500.f), 2000.f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 100000.f, false);
    trackb->Activate();
    trackb->GlueToEntity(cyl);
    AddView(trackb);
}