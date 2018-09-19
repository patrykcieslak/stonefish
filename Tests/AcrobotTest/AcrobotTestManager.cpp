//
//  AcrobotTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "AcrobotTestManager.h"

#include "AcrobotTestApp.h"
#include "SystemUtil.hpp"
#include "Plane.h"
#include "Box.h"
#include "Sphere.h"
#include "Torus.h"
#include "Cylinder.h"
#include "OpenGLPointLight.h"
#include "OpenGLTrackball.h"
#include "FixedJoint.h"
#include "RevoluteJoint.h"
#include "DCMotor.h"
#include "RotaryEncoder.h"
#include "FakeRotaryEncoder.h"
#include "ServoController.h"
#include "MISOStateSpaceController.h"
#include "Current.h"
#include "FeatherstoneEntity.h"
#include "Manipulator.h"
#include "Obstacle.h"
#include "ForceTorque.h"
#include "Light.h"
#include "Torque.h"

AcrobotTestManager::AcrobotTestManager(btScalar stepsPerSecond) : SimulationManager(UnitSystems::MKS, true, stepsPerSecond, SolverType::SI)
{
}

void AcrobotTestManager::BuildScenario()
{
    /////// BASICS
    OpenGLPipeline::getInstance()->setRenderingEffects(true, true, true);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, true, false, false, false);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    setGravity(9.81);
    setICSolverParams(false);
    
    /////// MATERIALS
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 4.0), 0.7);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MKS, 1.0), 0.3);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.01, 0.01);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.01, 0.01);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.01, 0.01);
    
    /////// LOOKS
    int grid = OpenGLContent::getInstance()->CreateSimpleLook(glm::vec3(1.f, 1.f, 1.f), 0.f, 0.1f, GetShaderPath() + "grid.png");
    int grey = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.7f, 0.7f, 0.7f), 0.5, 0.0);
	int shiny = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 0.3f, 0.3f), 0.3, 0.0);
    int green = OpenGLContent::getInstance()->CreatePhysicalLook(glm::vec3(0.3f, 1.0f, 0.3f), 0.1, 0.0);
    
    /////// OBJECTS
    Plane* floor = new Plane("Floor", 1000.f, getMaterialManager()->getMaterial("Concrete"), grid);
    AddStaticEntity(floor, btTransform::getIdentity());
    
    //Obstacle* box1 = new Obstacle("B1", btVector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    //AddStaticEntity(box1, btTransform(btQuaternion::getIdentity(), btVector3(0.0,-1.0,0.44)));
    /*Box* box2 = new Box("B2", btVector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Concrete"), shiny);
    
    FeatherstoneEntity* fe1 = new FeatherstoneEntity("FE1", 1, box1, getDynamicsWorld(), false);
    FeatherstoneEntity* fe2 = new FeatherstoneEntity("FE2", 1, box2, getDynamicsWorld(), false);
    AddFeatherstoneEntity(fe1, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0.5)));
    AddFeatherstoneEntity(fe2, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0.6)));
    
    FixedJoint* fix = new FixedJoint("Fix", fe1, fe2);
    AddJoint(fix);
    
    ForceTorque* ft = new ForceTorque("FT", fix, fe2->getLink(0).solid, btTransform::getIdentity());
    AddSensor(ft);*/
    
    Box* baseLink = new Box("BaseLink", btVector3(0.01,0.01,0.01), getMaterialManager()->getMaterial("Rubber"), shiny);
    Box* link1 = new Box("Link1", btVector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), shiny);
    link1->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link2 = new Box("Link2", btVector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), green);
    link2->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link3 = new Box("Link3", btVector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), grey);
    link3->ScalePhysicalPropertiesToArbitraryMass(1.0);
    
    Box* baseLinkB = new Box("BaseLinkB", btVector3(0.01,0.01,0.01), getMaterialManager()->getMaterial("Rubber"), shiny);
    Box* link1B = new Box("Link1B", btVector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), shiny);
    link1B->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link2B = new Box("Link2B", btVector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), green);
    link2B->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link3B = new Box("Link3B", btVector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), green);
    link3B->ScalePhysicalPropertiesToArbitraryMass(1.0);
    
    //Box* link3 = new Box("Link1", btVector3(0.1,0.1,0.5), getMaterialManager()->getMaterial("Rubber"), shiny);
    //link3->ScalePhysicalPropertiesToArbitraryMass(1.0);
    
    FeatherstoneEntity* fe = new FeatherstoneEntity("Manipulator1", 4, baseLink, getDynamicsWorld(), true);
    fe->AddLink(link1, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,-0.25,0)), getDynamicsWorld());
    fe->AddLink(link2, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,-0.75,0)), getDynamicsWorld());
    fe->AddLink(link3, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,-1.25,0)), getDynamicsWorld());
    
    //fe->AddRevoluteJoint(0, 1, btVector3(0,0,0), btVector3(1,0,0));
    //fe->AddRevoluteJoint(1, 2, btVector3(0,-0.5,0), btVector3(1,0,0));
    //fe->AddRevoluteJoint(2, 3, btVector3(0,-1.0,0), btVector3(1,0,0));
    fe->AddFixedJoint("Fix1", 0, 1, btVector3(0,0,0));
    fe->AddFixedJoint("Fix2", 1, 2, btVector3(0,-0.5,0));
    fe->AddFixedJoint("Fix3", 2, 3, btVector3(0,-1.0,0));
    
    /*fe->AddJointMotor(0, 10000.0);
    fe->MotorVelocitySetpoint(0, 0.0, 1.0);
    fe->MotorPositionSetpoint(0, 0.0, 1.0);
    
    fe->AddJointMotor(1, 10000.0);
    fe->MotorVelocitySetpoint(1, 0.0, 1.0);
    fe->MotorPositionSetpoint(1, 0.0, 1.0);
    
    fe->AddJointMotor(2, 10000.0);
    fe->MotorVelocitySetpoint(2, 0.0, 1.0);
    fe->MotorPositionSetpoint(2, 0.0, 1.0);
    */
    AddFeatherstoneEntity(fe, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0.5)));
    
    ForceTorque* ft = new ForceTorque("FT1", fe, 0, btTransform::getIdentity());
    AddSensor(ft);
    
    ft = new ForceTorque("FT2", fe, 1, btTransform::getIdentity());
    AddSensor(ft);
    
    ft = new ForceTorque("FT3", fe, 2, btTransform::getIdentity());
    AddSensor(ft);

    //Torque* tau = new Torque("Torque", fe, 1);
    //AddSensor(tau);
    
    FeatherstoneEntity* feB1 = new FeatherstoneEntity("Manipulator21", 2, baseLinkB, getDynamicsWorld(), true);
    feB1->AddLink(link1B, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,-0.25,0)), getDynamicsWorld());
    feB1->AddFixedJoint("Fix1", 0, 1, btVector3(0,0,0));
    //feB1->AddRevoluteJoint(0, 1, btVector3(0,0,0), btVector3(1,0,0));
    AddFeatherstoneEntity(feB1, btTransform(btQuaternion::getIdentity(), btVector3(0.1,0,0.5)));
    
    FeatherstoneEntity* feB2 = new FeatherstoneEntity("Manipulator22", 1, link2B, getDynamicsWorld(), false);
    AddFeatherstoneEntity(feB2, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0.1,-0.75,0.5)));
    
    FeatherstoneEntity* feB3 = new FeatherstoneEntity("Manipulator23", 1, link3B, getDynamicsWorld(), false);
    AddFeatherstoneEntity(feB3, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0.1,-1.25,0.5)));
    
    FixedJoint* ftFix = new FixedJoint("Fix", feB2, feB1, -1, 0, btVector3(0.1,-0.5,0.5));
    AddJoint(ftFix);
    ForceTorque* ft2 = new ForceTorque("FT4", ftFix, feB2->getLink(0).solid, btTransform::getIdentity());
    AddSensor(ft2);
    
    FixedJoint* ftFix2 = new FixedJoint("Fix2", feB3, feB2, -1, -1, btVector3(0.1,-1.0,0.5));
    AddJoint(ftFix2);
    ForceTorque* ft3 = new ForceTorque("FT5", ftFix2, feB3->getLink(0).solid, btTransform::getIdentity());
    AddSensor(ft3);
    
    /*Manipulator* manip = new Manipulator("Manipulator", 1, baseLink, btTransform::getIdentity());
    manip->AddRotLinkDH(link1, btTransform(btQuaternion(0,M_PI_2,1.5*M_PI_2), btVector3(0,0,-0.5)), 0, -1.0, 0.0, 1,-1, 10);
    //manip->AddRotLinkDH(link2, btTransform(btQuaternion(0,M_PI_2,M_PI_2), btVector3(0,0,-0.5)), 0, -1.0, 0.0);
    //manip->AddRotLinkDH(link3, btTransform(btQuaternion(0,M_PI_2,M_PI_2), btVector3(0,0,-0.25)), 0, -1.0, 0.0);
    AddSystemEntity(manip, btTransform(btQuaternion(0,0,M_PI_2), btVector3(0,0,0.5)));
    manip->SetDesiredJointVelocity(0, -0.5);*/
    //manip->SetDesiredJointPosition(0, 1.0);
    //manip->SetDesiredJointVelocity(2, -0.1);

    //Obstacle* sph = new Obstacle("Sphere", 1.0, getMaterialManager()->getMaterial("Concrete"), grey);
    //AddStaticEntity(sph, btTransform(btQuaternion::getIdentity(), btVector3(0,0,10)));
    
    //manip->SetDesiredJointVelocity(0,-0.1);
    
    
  /*  Plane* floor = new Plane("Floor", 20, getMaterialManager()->getMaterial("Concrete"), btTransform(btQuaternion(0,0,0), btVector3(0,0,-1.0)), grey);
    floor->setRenderable(true);
    AddEntity(floor);
    
#ifndef USE_FEATHERSTONE_ALGORITHM 
    /////// STANDARD APPROACH
    Box* arm1 = new Box("Arm1", btVector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm1, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.15)));
    arm1->SetArbitraryPhysicalProperties(2.5, btVector3(0.015, 0.015, 0.015), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.03)));
    
    Box* arm2 = new Box("Arm2", btVector3(0.03, 0.015, 0.13), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm2, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,0.085)));
    arm2->SetArbitraryPhysicalProperties(0.5, btVector3(0.001, 0.001, 0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.035)));
    
    RevoluteJoint* revo1 = new RevoluteJoint("Arm1Rotation", arm1, btVector3(0.0,0.0,0.0), btVector3(0.0,1.0,0.0));
    AddJoint(revo1);
    revo1->setDamping(0.0, 0.0);
    revo1->setIC(0.0);
     
    RevoluteJoint* revo2 = new RevoluteJoint("Arm2Rotation", arm2, arm1, btVector3(0.0,0.0,0.15), btVector3(0.0,1.0,0.0), false);
    AddJoint(revo2);
    revo2->setDamping(0.0, 0.0);
    revo2->setIC(0.0);
     
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Encoder1", revo1, 1000.0);
    AddSensor(enc1);
     
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Encoder2", revo2, 1000.0);
    AddSensor(enc2);
     
    DCMotor* motor = new DCMotor("DCX", revo2, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    AddActuator(motor);
#else
    /////// FEATHERSTONE APPROACH
    Box* base = new Box("Base", btVector3(0.5,0.5,0.05), getMaterialManager()->getMaterial("Rubber"), shiny);
    
    Box* arm1 = new Box("Arm1", btVector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), green);
    arm1->SetArbitraryPhysicalProperties(2.5, btVector3(0.015,0.015,0.015), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.03)));
    
    Box* arm2 = new Box("Arm2", btVector3(0.03, 0.015, 0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    arm2->SetArbitraryPhysicalProperties(0.5, btVector3(0.001,0.001,0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.05)));
    
	Box* arm3 = new Box("Arm3", btVector3(0.03, 0.015, 0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    arm3->SetArbitraryPhysicalProperties(0.5, btVector3(0.001,0.001,0.001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.05)));
    
    FeatherstoneEntity* fe = new FeatherstoneEntity("FE", 4, base, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,1.0)), getDynamicsWorld(), true);
    fe->setBaseRenderable(true);
    
    //Arm1
    fe->AddLink(arm1, btTransform(btQuaternion(0.0,0.0,0.0), btVector3(0.0,0.0,1.15)), getDynamicsWorld());
    fe->AddRevoluteJoint(0, 1, btVector3(0.0,0.0,1.0), btVector3(0.0, 1.0, 0.0), false);
    fe->setJointDamping(0, 0, 0.005);
    fe->setJointIC(0, UnitSystem::Angle(true, 1.0), UnitSystem::Angle(true, 0.0));
    
    //Arm2
	fe->AddLink(arm2, btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(0.0, -0.02, 1.1)), getDynamicsWorld());
    fe->AddRevoluteJoint(1, 2, btVector3(0.0, 0.0, 1.15), btVector3(0.0, 1.0, 0.0), false);
    fe->setJointDamping(1, 0.01, 0.005);
    
	//Arm3
	fe->AddLink(arm3, btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(0.0, -0.02, 1.2)), getDynamicsWorld());
    //fe->AddRevoluteJoint(1, 3, btVector3(0.0, 0.0, 1.15), btVector3(0.0, 1.0, 0.0), false);
    fe->AddFixedJoint(2, 3, btVector3(0.0, 0.0, 1.15));
	
    AddEntity(fe);
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Encoder1", fe, 0, 1000.0);
    AddSensor(enc1);
    
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Encoder2", fe, 0, 1000.0);
    AddSensor(enc2);
    
    DCMotor* motor = new DCMotor("DCX", fe, 1, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    AddActuator(motor);
#endif

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
    
    //MISOStateSpaceController* miso = new MISOStateSpaceController("Regulator", mux, motor, 12.0, 1000.0);
    //miso->SetGains(gains);
	//AddController(miso);
    
    //////CAMERA & LIGHT//////
    //OpenGLOmniLight* omni = new OpenGLOmniLight(btVector3(0,0,0), OpenGLLight::ColorFromTemperature(4000, 10));
    //omni->GlueToEntity(arm2);
    //AddLight(omni);
    
    //OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0.0f, 0.0f, 0.2f), 1.5f, btVector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 50.f, true);
    //trackb->Rotate(btQuaternion(M_PI - M_PI/8.0, 0.0, 0.0));
    //trackb->Activate();
    //AddView(trackb);
    
    Light* l = new Light("Spot", btVector3(0,0,1), glm::vec4(10.f,10.f,10.f,10.f));
    AddActuator(l);
}