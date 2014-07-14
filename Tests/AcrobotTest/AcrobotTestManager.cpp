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
    /////// BASICS
    OpenGLPipeline::getInstance()->setVisibleHelpers(false, false, false, false, false, true, true);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    setGravity(9.81);
    setICSolverParams(false);
    
    /////// MATERIALS
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 4.0), 0.7);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MKS, 2.0), 0.3);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.9, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.7);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.7, 0.5);
    
    /////// LOOKS
    Look grey = CreateOpaqueLook(glm::vec3(0.7f, 0.7f, 0.7f), 0.1, 0.8, 1.33);
    Look shiny = CreateOpaqueLook(glm::vec3(0.3f, 0.3f, 0.3f), 0.01, 0.9, 1.2);
    Look green = CreateOpaqueLook(glm::vec3(0.3f, 1.0f, 0.3f), 0.01, 0.9, 1.2);
    
    /////// OBJECTS
    PlaneEntity* floor = new PlaneEntity("Floor", 20, getMaterialManager()->getMaterial("Concrete"), grey, btTransform(btQuaternion(0,0,0), btVector3(0,0,-0.1)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    /////// STANDARD APPROACH
    //SphereEntity* sph = new SphereEntity("Sphere", 0.1, getMaterialManager()->getMaterial("Concrete"), grey);
    //AddSolidEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.2)));
    
    //BoxEntity* floor2 = new BoxEntity("Floor2", btVector3(1,1,0.1), getMaterialManager()->getMaterial("Concrete"), grey);
    //AddSolidEntity(floor2, btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.2)));
    
    /*
    BoxEntity* arm1 = new BoxEntity("Arm1", btVector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm1, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.15)));
    arm1->SetArbitraryPhysicalProperties(2.5, btVector3(0.015, 0.015, 0.015), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.03)));
    
    BoxEntity* arm2 = new BoxEntity("Arm2", btVector3(0.03, 0.015, 0.13), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm2, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.085)));
    arm2->SetArbitraryPhysicalProperties(0.5, btVector3(0.001, 0.001, 0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.035)));
    
    RevoluteJoint* revo1 = new RevoluteJoint("Arm1Rotation", arm1, btVector3(0.0,0.0,0.0), btVector3(0.0,1.0,0.0));
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
     
    DCMotor* motor = new DCMotor("DCX", revo2, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    //AddActuator(motor);
    */
    
    /////// FEATHERSTONE APPROACH
    BoxEntity* base = new BoxEntity("Base", btVector3(0.5,0.5,0.05), getMaterialManager()->getMaterial("Rubber"), shiny);
    //SphereEntity* base = new SphereEntity("Base", 0.05, getMaterialManager()->getMaterial("Rubber"), shiny);
    //CylinderEntity* base = new CylinderEntity("Cyl", 0.05, 0.1, getMaterialManager()->getMaterial("Rubber"), shiny);
    
    BoxEntity* arm1 = new BoxEntity("Arm1", btVector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), green);
    arm1->SetArbitraryPhysicalProperties(2.5, btVector3(0.015,0.015,0.015), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.03)));
    
    BoxEntity* arm2 = new BoxEntity("Arm2", btVector3(0.03, 0.015, 0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    arm2->SetArbitraryPhysicalProperties(0.5, btVector3(0.001,0.001,0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.05)));
    
    FeatherstoneEntity* fe = new FeatherstoneEntity("FE", 3, base, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.0)), getDynamicsWorld(), true);
    fe->setBaseRenderable(false);
    
    //Arm1
    fe->AddLink(arm1, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.15)), getDynamicsWorld());
    fe->AddRevoluteJoint(0, 1, btVector3(0.0,0.0,0.0), btVector3(0.0, 1.0, 0.0), false);
    fe->setJointIC(0, UnitSystem::Angle(true, 3.0), UnitSystem::Angle(true, 0.0));
    //fe->setJointDamping(0, 0, 0.005);
    
    //Arm2
    fe->AddLink(arm2, btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(0.0, -0.02, 0.1)), getDynamicsWorld());
    fe->AddRevoluteJoint(1, 2, btVector3(0.0, 0.0, 0.15), btVector3(0.0, 1.0, 0.0), false);
    //fe->setJointDamping(1, 0.01, 0.005);
    
    AddEntity(fe);
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Encoder1", fe, 0, 1000.0);
    AddSensor(enc1);
    
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Encoder2", fe, 1, 1000.0);
    AddSensor(enc2);
    
    DCMotor* motor = new DCMotor("DCX", fe, 1, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    AddActuator(motor);
    
    /////// COMMON
    FakeIMU* imu = new FakeIMU("IMU", arm1, btTransform::getIdentity(), 1000.0);
    AddSensor(imu);
     
    Current* cur = new Current("Current", motor, 1000.0);
    AddSensor(cur);
    
    Mux* mux = new Mux();
    mux->AddComponent(enc1, 0);
    mux->AddComponent(enc2, 0);
    mux->AddComponent(enc1, 1);
    mux->AddComponent(enc2, 1);
    mux->AddComponent(cur, 0);
    
    
    //LFcombo = -42.37441    4.78974   -5.50932    0.35351   -0.18878 [theta alpha dtheta dalpha current]
    //Sampling: 1/1000  Theta0: 3 deg
    //LFcombo(500Hz) =  -75.55122    8.53945   -9.82255    0.64889  -0.17052
    
    
    std::vector<btScalar> gains;
    gains.push_back(-42.37441);
    gains.push_back(4.78974);
    gains.push_back(-5.50932);
    gains.push_back(0.35351);
    gains.push_back(-0.18878);
    /*gains.push_back(-75.55122);
    gains.push_back(8.53945);
    gains.push_back(-9.82255);
    gains.push_back(0.64889);
    gains.push_back(-0.17052);*/
    
    MISOStateSpaceController* miso = new MISOStateSpaceController("Regulator", mux, motor, 12.0, 1000.0);
    miso->SetGains(gains);
    AddController(miso);
    
    //////CAMERA & LIGHT//////
    //OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(0,0,0), OpenGLLight::ColorFromTemperature(4000, 10));
    //omni->GlueToEntity(arm2);
    //AddLight(omni);
    
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0.0f, 0.0f, 0.2f), 1.5f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 50.f, false);
    trackb->Rotate(btQuaternion(M_PI - M_PI/8.0, 0.0, 0.0));
    trackb->Activate();
    AddView(trackb);
}