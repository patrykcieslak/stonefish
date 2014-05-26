//
//  AcrobotTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcrobotTestManager.h"

#include "AcrobotTestApp.h"
#include "PlaneEntity.h"
#include "BoxEntity.h"
#include "TorusEntity.h"
#include "CylinderEntity.h"
#include "OpenGLOmniLight.h"
#include "OpenGLTrackball.h"
#include "FixedJoint.h"
#include "RevoluteJoint.h"
#include "FakeIMU.h"
#include "DCMotor.h"
#include "RotaryEncoder.h"
#include "FakeRotaryEncoder.h"
#include "ServoController.h"

AcrobotTestManager::AcrobotTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, true, stepsPerSecond, DANTZIG, INCLUSIVE)
{
}

void AcrobotTestManager::BuildScenario()
{
    //--------------------Using MSK unit system--------------------
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 4.0), 0.7);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MKS, 7.81), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.6, 0.3);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.7, 0.5);
    
    ///////LOOKS///////////
    Look grey = CreateMatteLook(0.7f, 0.7f, 0.7f, 0.0f);
    Look shiny = CreateMatteLook(0.3f, 0.3f, 0.3f, 0.5f);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 20.f, getMaterialManager()->getMaterial("Concrete"), grey, btTransform(btQuaternion(0,0,0), btVector3(0,0,-0.5f)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    BoxEntity* arm1 = new BoxEntity("Arm1", btVector3(0.04f, 0.02f, 0.3f), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm1, btTransform(btQuaternion::getIdentity(), btVector3(0.f,0.f,0.15f)));
    arm1->SetArbitraryPhysicalProperties(2.5, btVector3(0.015, 0.015, 0.015), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.03f)));
    
    BoxEntity* arm2 = new BoxEntity("Arm2", btVector3(0.03f, 0.015f, 0.13f), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm2, btTransform(btQuaternion::getIdentity(), btVector3(0.f,0.02f,0.085f)));
    arm2->SetArbitraryPhysicalProperties(0.5, btVector3(0.001, 0.001, 0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.035f)));
    
    RevoluteJoint* revo1 = new RevoluteJoint("Arm1Rotation", arm1, btVector3(0.001f,0.0f,0.f), btVector3(0.f,1.f,0.f));
    AddJoint(revo1);
    revo1->setDamping(0.0, 0.001);
    
    RevoluteJoint* revo2 = new RevoluteJoint("Arm2Rotation", arm2, arm1, btVector3(0.f,0.f,0.15f), btVector3(0.f,1.f,0.f), false);
    AddJoint(revo2);
    revo2->setDamping(0.0, 0.001);
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Arm1Encoder", revo1, 1000000);
    AddSensor(enc1);
    
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Arm2Encoder", revo2, 1000000);
    AddSensor(enc2);
    
    FakeIMU* imu = new FakeIMU("IMU", arm1, btTransform::getIdentity(), 1000000);
    AddSensor(imu);
    
    DCMotor* motor = new DCMotor("DCX", revo2, 0.212f, 0.0774e-3, 1.f/408.f, 23.4e-3, 0.0000055f);
    AddActuator(motor);
    
/*    ServoController* srv = new ServoController("ServoController", motor, enc2, 5.0, 500);
    AddController(srv);
    srv->SetPosition(M_PI);
    srv->SetGains(3.0, 0.0, 0.0);*/
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0.0f, 0.0f, 0.0f), 1.f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 50.f, false);
    trackb->Rotate(btQuaternion(0.0, 0.0, -M_PI_2/9.f) * btQuaternion(-M_PI_4,0.0,0.0));
    trackb->Activate();
    AddView(trackb);
}