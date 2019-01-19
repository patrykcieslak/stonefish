//
//  AcrobotTestManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "AcrobotTestManager.h"

#include "AcrobotTestApp.h"
#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLTrackball.h>
#include <joints/FixedJoint.h>
#include <joints/RevoluteJoint.h>
#include <actuators/DCMotor.h>
#include <controllers/ServoController.h>
#include <controllers/MISOStateSpaceController.h>
#include <entities/FeatherstoneEntity.h>
#include <entities/statics/Obstacle.h>
#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>

AcrobotTestManager::AcrobotTestManager(sf::Scalar stepsPerSecond)
    : SimulationManager(stepsPerSecond, sf::SolverType::SOLVER_SI, sf::CollisionFilteringType::COLLISION_EXCLUSIVE, sf::HydrodynamicsType::GEOMETRY_BASED)
{
}

void AcrobotTestManager::BuildScenario()
{
    /////// BASICS
    setGravity(1.0);
    setICSolverParams(false);
    
    /////// MATERIALS
    getMaterialManager()->CreateMaterial("Concrete", sf::UnitSystem::Density(sf::CGS, sf::MKS, 4.0), 0.7);
    getMaterialManager()->CreateMaterial("Rubber", sf::UnitSystem::Density(sf::CGS, sf::MKS, 1.0), 0.3);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.01, 0.01);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.01, 0.01);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.01, 0.01);
    
    /////// LOOKS
    int grid = CreateLook(sf::Color::RGB(1.f, 1.f, 1.f), 0.f, 0.1f, 0.f, sf::GetShaderPath() + "grid.png");
    int grey = CreateLook(sf::Color::RGB(0.7f, 0.7f, 0.7f), 0.5, 0.0);
    int shiny = CreateLook(sf::Color::RGB(0.3f, 0.3f, 0.3f), 0.3, 0.0);
    int green = CreateLook(sf::Color::RGB(0.3f, 1.0f, 0.3f), 0.1, 0.0);
    
    /////// OBJECTS
    sf::Plane* floor = new sf::Plane("Floor", 1000.f, getMaterialManager()->getMaterial("Concrete"), grid);
    AddStaticEntity(floor, sf::I4());
    
    //Obstacle* box1 = new Obstacle("B1", Vector3(0.5,0.1,0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    //AddStaticEntity(box1, Transform(Quaternion::getIdentity(), Vector3(0.0,-1.0,0.44)));
    /*Box* box2 = new Box("B2", Vector3(0.1,0.1,0.1), getMaterialManager()->getMaterial("Concrete"), shiny);
    
    FeatherstoneEntity* fe1 = new FeatherstoneEntity("FE1", 1, box1, getDynamicsWorld(), false);
    FeatherstoneEntity* fe2 = new FeatherstoneEntity("FE2", 1, box2, getDynamicsWorld(), false);
    AddFeatherstoneEntity(fe1, Transform(Quaternion(0,0,M_PI_2), Vector3(0,0,0.5)));
    AddFeatherstoneEntity(fe2, Transform(Quaternion(0,0,M_PI_2), Vector3(0,0,0.6)));
    
    FixedJoint* fix = new FixedJoint("Fix", fe1, fe2);
    AddJoint(fix);
    
    ForceTorque* ft = new ForceTorque("FT", fix, fe2->getLink(0).solid, Transform::getIdentity());
    AddSensor(ft);*/
    
    sf::Box* baseLink = new sf::Box("BaseLink", sf::Vector3(0.01,0.01,0.01), sf::I4(), getMaterialManager()->getMaterial("Rubber"), shiny);
    sf::Box* link1 = new sf::Box("Link1", sf::Vector3(0.01,0.01,0.5), sf::I4(), getMaterialManager()->getMaterial("Rubber"), shiny);
    sf::Box* link2 = new sf::Box("Link2", sf::Vector3(0.01,0.01,0.5), sf::I4(), getMaterialManager()->getMaterial("Rubber"), shiny);
    
    /*link1->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link2 = new Box("Link2", Vector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), green);
    link2->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link3 = new Box("Link3", Vector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), grey);
    link3->ScalePhysicalPropertiesToArbitraryMass(1.0);*/
    
    /*Box* baseLinkB = new Box("BaseLinkB", Vector3(0.01,0.01,0.01), getMaterialManager()->getMaterial("Rubber"), shiny);
    Box* link1B = new Box("Link1B", Vector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), shiny);
    link1B->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link2B = new Box("Link2B", Vector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), green);
    link2B->ScalePhysicalPropertiesToArbitraryMass(1.0);
    Box* link3B = new Box("Link3B", Vector3(0.01,0.01,0.5), getMaterialManager()->getMaterial("Rubber"), green);
    link3B->ScalePhysicalPropertiesToArbitraryMass(1.0);
    
    //Box* link3 = new Box("Link1", Vector3(0.1,0.1,0.5), getMaterialManager()->getMaterial("Rubber"), shiny);
    //link3->ScalePhysicalPropertiesToArbitraryMass(1.0);
    */
    
    sf::FeatherstoneEntity* fe = new sf::FeatherstoneEntity("Manipulator1", 3, baseLink, true);
    fe->AddLink(link1, sf::Transform(sf::Quaternion(0,0,M_PI_2), sf::Vector3(0,-0.25,0)));
    fe->AddLink(link2, sf::Transform(sf::Quaternion(0,0,M_PI_2), sf::Vector3(0,-0.75,0)));
    
    //fe->AddLink(link2, Transform(Quaternion(0,0,M_PI_2), Vector3(0,-0.75,0)));
    //fe->AddLink(link3, Transform(Quaternion(0,0,M_PI_2), Vector3(0,-1.25,0)));
    
    fe->AddRevoluteJoint("Joint1", 0, 1, sf::Vector3(0,0,0), sf::Vector3(1,0,0));
    //fe->AddRevoluteJoint(1, 2, Vector3(0,-0.5,0), Vector3(1,0,0));
    //fe->AddRevoluteJoint(2, 3, Vector3(0,-1.0,0), Vector3(1,0,0));
    //fe->AddFixedJoint("Fix1", 0, 1, Vector3(0,0,0));
    fe->AddFixedJoint("Fix2", 1, 2, sf::Vector3(0,-0.5,0));
    //fe->AddFixedJoint("Fix3", 2, 3, Vector3(0,-1.0,0));
    //fe->AddJointMotor(0, 10000.0);
    //fe->MotorVelocitySetpoint(0, 0.0, 1.0);
    //fe->MotorPositionSetpoint(0, 0.0, 1.0);
    /*
    fe->AddJointMotor(1, 10000.0);
    fe->MotorVelocitySetpoint(1, 0.0, 1.0);
    fe->MotorPositionSetpoint(1, 0.0, 1.0);
    
    fe->AddJointMotor(2, 10000.0);
    fe->MotorVelocitySetpoint(2, 0.0, 1.0);
    fe->MotorPositionSetpoint(2, 0.0, 1.0);
    */
    
    AddFeatherstoneEntity(fe, sf::Transform(sf::Quaternion::getIdentity(), sf::Vector3(0,0,-3.0)));
    
    /*
    ForceTorque* ft = new ForceTorque("FT1", fe, 0, Transform::getIdentity());
    AddSensor(ft);
    
    ft = new ForceTorque("FT2", fe, 1, Transform::getIdentity());
    AddSensor(ft);
    
    ft = new ForceTorque("FT3", fe, 2, Transform::getIdentity());
    AddSensor(ft);

    //Torque* tau = new Torque("Torque", fe, 1);
    //AddSensor(tau);
    
    FeatherstoneEntity* feB1 = new FeatherstoneEntity("Manipulator21", 2, baseLinkB, true);
    feB1->AddLink(link1B, Transform(Quaternion(0,0,M_PI_2), Vector3(0,-0.25,0)));
    feB1->AddFixedJoint("Fix1", 0, 1, Vector3(0,0,0));
    //feB1->AddRevoluteJoint(0, 1, Vector3(0,0,0), Vector3(1,0,0));
    AddFeatherstoneEntity(feB1, Transform(Quaternion::getIdentity(), Vector3(0.1,0,0.5)));
    
    FeatherstoneEntity* feB2 = new FeatherstoneEntity("Manipulator22", 1, link2B, false);
    AddFeatherstoneEntity(feB2, Transform(Quaternion(0,0,M_PI_2), Vector3(0.1,-0.75,0.5)));
    
    FeatherstoneEntity* feB3 = new FeatherstoneEntity("Manipulator23", 1, link3B, false);
    AddFeatherstoneEntity(feB3, Transform(Quaternion(0,0,M_PI_2), Vector3(0.1,-1.25,0.5)));
    
    FixedJoint* ftFix = new FixedJoint("Fix", feB2, feB1, -1, 0, Vector3(0.1,-0.5,0.5));
    AddJoint(ftFix);
    ForceTorque* ft2 = new ForceTorque("FT4", ftFix, feB2->getLink(0).solid, Transform::getIdentity());
    AddSensor(ft2);
    
    FixedJoint* ftFix2 = new FixedJoint("Fix2", feB3, feB2, -1, -1, Vector3(0.1,-1.0,0.5));
    AddJoint(ftFix2);
    ForceTorque* ft3 = new ForceTorque("FT5", ftFix2, feB3->getLink(0).solid, Transform::getIdentity());
    AddSensor(ft3);
    */
    /*Manipulator* manip = new Manipulator("Manipulator", 1, baseLink, Transform::getIdentity());
    manip->AddRotLinkDH(link1, Transform(Quaternion(0,M_PI_2,1.5*M_PI_2), Vector3(0,0,-0.5)), 0, -1.0, 0.0, 1,-1, 10);
    //manip->AddRotLinkDH(link2, Transform(Quaternion(0,M_PI_2,M_PI_2), Vector3(0,0,-0.5)), 0, -1.0, 0.0);
    //manip->AddRotLinkDH(link3, Transform(Quaternion(0,M_PI_2,M_PI_2), Vector3(0,0,-0.25)), 0, -1.0, 0.0);
    AddSystemEntity(manip, Transform(Quaternion(0,0,M_PI_2), Vector3(0,0,0.5)));
    manip->SetDesiredJointVelocity(0, -0.5);*/
    //manip->SetDesiredJointPosition(0, 1.0);
    //manip->SetDesiredJointVelocity(2, -0.1);

    //Obstacle* sph = new Obstacle("Sphere", 1.0, getMaterialManager()->getMaterial("Concrete"), grey);
    //AddStaticEntity(sph, Transform(Quaternion::getIdentity(), Vector3(0,0,10)));
    
    //manip->SetDesiredJointVelocity(0,-0.1);
    
    
  /*  Plane* floor = new Plane("Floor", 20, getMaterialManager()->getMaterial("Concrete"), Transform(Quaternion(0,0,0), Vector3(0,0,-1.0)), grey);
    floor->setRenderable(true);
    AddEntity(floor);
    
#ifndef USE_FEATHERSTONE_ALGORITHM 
    /////// STANDARD APPROACH
    Box* arm1 = new Box("Arm1", Vector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm1, Transform(Quaternion(0.0,0.0,0.0), Vector3(0.0,0.0,0.15)));
    arm1->SetArbitraryPhysicalProperties(2.5, Vector3(0.015, 0.015, 0.015), Transform(Quaternion::getIdentity(), Vector3(0,0,-0.03)));
    
    Box* arm2 = new Box("Arm2", Vector3(0.03, 0.015, 0.13), getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(arm2, Transform(Quaternion(0.0,0.0,0.0), Vector3(0.0,0.0,0.085)));
    arm2->SetArbitraryPhysicalProperties(0.5, Vector3(0.001, 0.001, 0.001), Transform(Quaternion::getIdentity(), Vector3(0,0,-0.035)));
    
    RevoluteJoint* revo1 = new RevoluteJoint("Arm1Rotation", arm1, Vector3(0.0,0.0,0.0), Vector3(0.0,1.0,0.0));
    AddJoint(revo1);
    revo1->setDamping(0.0, 0.0);
    revo1->setIC(0.0);
     
    RevoluteJoint* revo2 = new RevoluteJoint("Arm2Rotation", arm2, arm1, Vector3(0.0,0.0,0.15), Vector3(0.0,1.0,0.0), false);
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
    Box* base = new Box("Base", Vector3(0.5,0.5,0.05), getMaterialManager()->getMaterial("Rubber"), shiny);
    
    Box* arm1 = new Box("Arm1", Vector3(0.04, 0.02, 0.3), getMaterialManager()->getMaterial("Rubber"), green);
    arm1->SetArbitraryPhysicalProperties(2.5, Vector3(0.015,0.015,0.015), Transform(Quaternion::getIdentity(), Vector3(0,0,-0.03)));
    
    Box* arm2 = new Box("Arm2", Vector3(0.03, 0.015, 0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    arm2->SetArbitraryPhysicalProperties(0.5, Vector3(0.001,0.001,0.001), Transform(Quaternion::getIdentity(), Vector3(0,0,-0.05)));
    
	Box* arm3 = new Box("Arm3", Vector3(0.03, 0.015, 0.1), getMaterialManager()->getMaterial("Rubber"), shiny);
    arm3->SetArbitraryPhysicalProperties(0.5, Vector3(0.001,0.001,0.001), Transform(Quaternion::getIdentity(), Vector3(0,0,-0.05)));
    
    FeatherstoneEntity* fe = new FeatherstoneEntity("FE", 4, base, Transform(Quaternion(0.0,0.0,0.0), Vector3(0.0,0.0,1.0)), getDynamicsWorld(), true);
    fe->setBaseRenderable(true);
    
    //Arm1
    fe->AddLink(arm1, Transform(Quaternion(0.0,0.0,0.0), Vector3(0.0,0.0,1.15)), getDynamicsWorld());
    fe->AddRevoluteJoint(0, 1, Vector3(0.0,0.0,1.0), Vector3(0.0, 1.0, 0.0), false);
    fe->setJointDamping(0, 0, 0.005);
    fe->setJointIC(0, UnitSystem::Angle(true, 1.0), UnitSystem::Angle(true, 0.0));
    
    //Arm2
	fe->AddLink(arm2, Transform(Quaternion(0.0, 0.0, 0.0), Vector3(0.0, -0.02, 1.1)), getDynamicsWorld());
    fe->AddRevoluteJoint(1, 2, Vector3(0.0, 0.0, 1.15), Vector3(0.0, 1.0, 0.0), false);
    fe->setJointDamping(1, 0.01, 0.005);
    
	//Arm3
	fe->AddLink(arm3, Transform(Quaternion(0.0, 0.0, 0.0), Vector3(0.0, -0.02, 1.2)), getDynamicsWorld());
    //fe->AddRevoluteJoint(1, 3, Vector3(0.0, 0.0, 1.15), Vector3(0.0, 1.0, 0.0), false);
    fe->AddFixedJoint(2, 3, Vector3(0.0, 0.0, 1.15));
	
    AddEntity(fe);
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("Encoder1", fe, 0, 1000.0);
    AddSensor(enc1);
    
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("Encoder2", fe, 0, 1000.0);
    AddSensor(enc2);
    
    DCMotor* motor = new DCMotor("DCX", fe, 1, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    AddActuator(motor);
#endif

    /////// COMMON
    FakeIMU* imu = new FakeIMU("IMU", arm1, Transform::getIdentity(), 1000.0);
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
    
    std::vector<Scalar> gains;
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
    //OpenGLOmniLight* omni = new OpenGLOmniLight(Vector3(0,0,0), OpenGLLight::ColorFromTemperature(4000, 10));
    //omni->GlueToEntity(arm2);
    //AddLight(omni);
    
    //OpenGLTrackball* trackb = new OpenGLTrackball(Vector3(0.0f, 0.0f, 0.2f), 1.5f, Vector3(0,0,1.f), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 50.f, true);
    //trackb->Rotate(Quaternion(M_PI - M_PI/8.0, 0.0, 0.0));
    //trackb->Activate();
    //AddView(trackb);
}
