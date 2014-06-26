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
#include "SphereEntity.h"
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
#include "MISOStateSpaceController.h"
#include "Current.h"
#include "FeatherstoneEntity.h"

AcrobotTestManager::AcrobotTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, true, stepsPerSecond, DANTZIG, STANDARD)
{
}

void AcrobotTestManager::BuildScenario()
{
    OpenGLPipeline::getInstance()->SetVisibleElements(true, true, false, false, false);
    //--------------------Using MSK unit system--------------------
    setGravity(9.81);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 4.0), 0.7);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MKS, 7.81), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.9, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.7, 0.5);
    
    ///////LOOKS///////////
    Look grey = CreateOpaqueLook(glm::vec3(0.7f, 0.7f, 0.7f), 0.1, 0.8, 1.33);
    Look shiny = CreateOpaqueLook(glm::vec3(0.3f, 0.3f, 0.3f), 0.01, 0.9, 1.2);
    Look green = CreateOpaqueLook(glm::vec3(0.3f, 1.0f, 0.3f), 0.01, 0.9, 1.2);
    
    ////////OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 20, getMaterialManager()->getMaterial("Concrete"), grey, btTransform(btQuaternion(0,0,0), btVector3(0,0,-0.4)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    //SphereEntity* sphere = new SphereEntity("Sphere", 0.2, getMaterialManager()->getMaterial("Rubber"), shiny);
    //AddSolidEntity(sphere, btTransform(btQuaternion::getIdentity(), btVector3(0.1, 0.0, -0.2)));
    
    //BoxEntity* arm1 = new BoxEntity("Arm1", btVector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), shiny);
    //AddSolidEntity(arm1, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.15)));
    //arm1->SetArbitraryPhysicalProperties(2.5, btVector3(0.015, 0.015, 0.015), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.03)));
    
    //BoxEntity* arm2 = new BoxEntity("Arm2", btVector3(0.03, 0.015, 0.13), getMaterialManager()->getMaterial("Rubber"), shiny);
    //AddSolidEntity(arm2, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.085)));
    //arm2->SetArbitraryPhysicalProperties(0.5, btVector3(0.001, 0.001, 0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.035)));
    
    BoxEntity* base = new BoxEntity("Base", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    base->SetArbitraryPhysicalProperties(10.0, btVector3(1,1,1), btTransform(btQuaternion::getIdentity(), btVector3(0,0,0.025)));
    
    BoxEntity* arm1 = new BoxEntity("Arm1", btVector3(0.05, 0.15, 0.4), getMaterialManager()->getMaterial("Rubber"), green);
    arm1->SetArbitraryPhysicalProperties(3.0, btVector3(0.01,0.01,0.01), btTransform(btQuaternion::getIdentity(), btVector3(0,0,0.1))); // 1/4 height
    
    FeatherstoneEntity* fe = new FeatherstoneEntity("FE", 2, base, btTransform(btQuaternion::getIdentity(), btVector3(0.0, 0.0, 0.05)), getDynamicsWorld(), true);
    fe->setBaseRenderable(true);
    fe->AddLink(arm1, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.2)), getDynamicsWorld());
    //fe->AddLink(arm2, btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.085)), getDynamicsWorld());
    fe->AddRevoluteJoint(0, 1, btVector3(0.0, 0.0, 0.0), btVector3(0.0, 1.0, 0.0), false);
    //fe->AddRevoluteJoint(1, 2, btVector3(0.0, 0.0, 0.15), btVector3(0.0, 1.0, 0.0), false);
    //fe->setJointIC(1, 0.0, 0.0);
    AddEntity(fe);
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Encoder1", fe, 1);
    AddSensor(enc1);
    
    //FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Encoder2", fe, 2);
    //AddSensor(enc2);
    
    //DCMotor* motor = new DCMotor("DCX", fe, 2, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    //motor->setVoltage(0.1);
    //AddActuator(motor);
    
    /*RevoluteJoint* revo1 = new RevoluteJoint("Arm1Rotation", arm1, btVector3(0.0,0.0,0.0), btVector3(0.0,1.0,0.0));
    AddJoint(revo1);
    revo1->setDamping(0.0, 0.0);
    revo1->setIC(1.0, 0.0);
    
    RevoluteJoint* revo2 = new RevoluteJoint("Arm2Rotation", arm2, arm1, btVector3(0.0,0.0,0.15), btVector3(0.0,1.0,0.0), false);
    AddJoint(revo2);
    revo2->setDamping(0.0, 0.0);
    revo2->setIC(0.0, 0.0);
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Encoder1", revo1, 100000);
    AddSensor(enc1);
    
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Encoder2", revo2, 100000);
    AddSensor(enc2);
    
   // FakeIMU* imu = new FakeIMU("IMU", arm1, btTransform::getIdentity(), 1000000);
   // AddSensor(imu);
    
    //DCMotor* motor = new DCMotor("DCX", revo2, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    //AddActuator(motor);
    */
    
    /*Current* cur = new Current("Current", motor);
    AddSensor(cur);
    
    Mux* mux = new Mux();
    mux->AddComponent(enc1, 0);
    mux->AddComponent(enc2, 0);
    mux->AddComponent(enc1, 1);
    mux->AddComponent(enc2, 1);
    mux->AddComponent(cur, 0);
    
    std::vector<btScalar> gains;
    gains.push_back(-42.3737);
    gains.push_back(4.7897);
    gains.push_back(-5.5093);
    gains.push_back(0.3535);
    gains.push_back(-0.1888);
    
    MISOStateSpaceController* miso = new MISOStateSpaceController("Regulator", mux, motor, 12.0);
    miso->SetGains(gains);
    AddController(miso);*/
    
    //LFcombo = -42.3737    4.7897   -5.5093    0.3535   -0.1888 [theta alpha dtheta dalpha current]
    //Sampling: 1/1000  Theta0: 3 deg
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0.0f, 0.0f, 0.2f), 3.f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 50.f, false);
    trackb->Rotate(btQuaternion(M_PI, 0.0, 0.0));
    trackb->Activate();
    AddView(trackb);
}