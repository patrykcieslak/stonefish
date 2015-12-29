//
//  AcroverSimManager.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcroverSimManager.h"

#include "AcroverSimApp.h"
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
#include "FakeRotaryEncoder.h"
#include "Current.h"
#include "SystemUtil.h"
#include "Motor.h"
#include "Trajectory.h"
#include "PathGenerator2D.h"
#include "FeatherstoneEntity.h"
#include "SignalGenerator.h"

#include "AcroverDriveController.h"
#include "AcroverSpeedController.h"
#include "AcroverTiltController.h"
#include "AcroverPathFollowingController.h"

AcroverSimManager::AcroverSimManager(btScalar stepsPerSecond) : SimulationManager(MKS, true, stepsPerSecond, DANTZIG, INCLUSIVE)
{
}

void AcroverSimManager::BuildScenario()
{
    //--------------------Using MSK unit system--------------------
    setICSolverParams(true);
    
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    OpenGLPipeline::getInstance()->setVisibleHelpers(true, false, false, false, false, false, false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 4.0), 0.2);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MKS, 2.0), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 100.0, 100.0);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.8);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.8, 0.7);
    
    ///////LOOKS///////////
    char path[1024];
    GetCWD(path, 1024);
    GetDataPath(path, 1024-32);
    strcat(path, "grid.png");
    
    Look grid = CreateOpaqueLook(glm::vec3(1.f, 1.f, 1.f), 0.5f, 0.8f, 1.7f, path);
    Look grey = CreateOpaqueLook(glm::vec3(0.7f, 0.7f, 0.7f), 0.5f, 0.5f, 1.3f);
    
    GetCWD(path, 1024);
    GetDataPath(path, 1024-32);
    strcat(path, "checker.png");
    Look shiny = CreateOpaqueLook(glm::vec3(0.1f, 0.1f, 0.1f), 0.3f, 0.1f, 1.2f, path, 0.2f);
    
    Look orange = CreateOpaqueLook(glm::vec3(0.8f, 0.5f, 0.1f), 0.5f, 0.4f, 1.5f);
    
    /////////////OBJECTS//////////////
    PlaneEntity* floor = new PlaneEntity("Floor", 200.f, getMaterialManager()->getMaterial("Concrete"), grid, btTransform(btQuaternion(0,0,0), btVector3(0,0,0)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    //Wheel
    //SphereEntity* torus = new SphereEntity("Wheel", 0.02, getMaterialManager()->getMaterial("Rubber"), shiny);
    btScalar Rt = 0.240;
    btScalar rt = 0.015;
    TorusEntity* torus = new TorusEntity("Wheel", Rt - rt, rt, getMaterialManager()->getMaterial("Rubber"), shiny);
    torus->SetArbitraryPhysicalProperties(0.577, btVector3(0.013, 0.025, 0.013), btTransform::getIdentity());
    
    //Cart
    CylinderEntity* cyl = new CylinderEntity("Cart", 0.2, 0.01, getMaterialManager()->getMaterial("Concrete"), grey);
    cyl->SetArbitraryPhysicalProperties(3.668, btVector3(0.029, 0.067, 0.04), btTransform(btQuaternion::getIdentity(), btVector3(0.0, 0.0, -0.026)));
    
    //Lever
    BoxEntity* box = new BoxEntity("Lever", btVector3(0.1, 0.03, 0.2), getMaterialManager()->getMaterial("Concrete"), orange);
    box->SetArbitraryPhysicalProperties(1.361, btVector3(0.005, 0.006, 0.003), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.028)));
    
    //Robot
    FeatherstoneEntity* mwr = new FeatherstoneEntity("Mono-wheel robot", 3, torus, btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0.3)), getDynamicsWorld(), false);
    mwr->setBaseRenderable(true);
    mwr->AddLink(cyl, btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0.3)), getDynamicsWorld());
    mwr->AddRevoluteJoint(0, 1, btVector3(0, 0, 0.3), btVector3(0,1,0));
    mwr->AddLink(box, btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0.3 + 0.014 - 0.1)), getDynamicsWorld());
    mwr->AddRevoluteJoint(1, 2, btVector3(0, 0, 0.3 + 0.014), btVector3(1,0,0));
    
    mwr->setJointIC(1, 0.0, 0.0);
    mwr->setJointDamping(1, 0.0, 0.01);
    mwr->setJointDamping(0, 0.01, 0.01);
    AddEntity(mwr);
    
    Contact* c = AddContact(torus, floor, 1000000);
    c->setDisplayMask(CONTACT_DISPLAY_PATH_A);
    
    unsigned int sensorHistory = 1000000;
    
    //DRIVES
    DCMotor* motor1 = new DCMotor("MotorWheel", mwr, 0, 0.165, 0.0525e-3, 1.0/490.0, 19.5e-3, 0.0);
    motor1->SetupGearbox(true, 22.44, 1.0);
    AddActuator(motor1);
    
    DCMotor* motor2 = new DCMotor("MotorPendulum", mwr, 1, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    motor2->SetupGearbox(true, (48.0/12.0)*(44.0/15.0), 1.0); //11.73
    AddActuator(motor2);
    
    //SENSORS
    FakeIMU* imu = new FakeIMU("AHRS", cyl, btTransform::getIdentity(), -1, sensorHistory);
    AddSensor(imu);
    //0 -> Theta (Roll)
    //1 -> Psi (Pitch)
    //2 -> Phi (Yaw)
    
    FakeRotaryEncoder* enc1 = new FakeRotaryEncoder("EncoderWheel", mwr, 0, -1, sensorHistory);
    AddSensor(enc1);
    
    FakeRotaryEncoder* enc2 = new FakeRotaryEncoder("EncoderPendulum", mwr, 1, -1, sensorHistory);
    AddSensor(enc2);
    
    Trajectory* traj = new Trajectory("Trajectory", cyl, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), -1, sensorHistory);
    traj->setRenderable(true);
    AddSensor(traj);
    
    //////////////CONTROL///////////////
    //--------Longitudinal---------
    AcroverDriveController* drive1 = new AcroverDriveController("Drive1", motor1, enc1, 2.0, 16.5, 500.0);
    drive1->SetGains(0.5, 150.0);
    
    AcroverSpeedController* longitudinal = new AcroverSpeedController("SpeedController", imu, enc1, drive1, 250.0);
    
    //-----------Lateral------------
    AcroverDriveController* drive2 = new AcroverDriveController("Drive2", motor2, enc2, 5.0, 16.5, 500.0);
    drive2->SetGains(0.5, 150.0);
    
    AcroverTiltController* lateral = new AcroverTiltController("TiltController", imu, enc2, enc1, drive2, 250.0);
    
    ////////////PRESCRIBED PATH
    PathGenerator2D* pg2d = new PathGenerator2D(PLANE_XY);
    pg2d->setRenderable(true);
    
    /*Pwl2D* pwl1 = new Pwl2D(Point2D(0.0, 0.0));
    pwl1->AddLineToPoint(Point2D(2.0, 0.0));
    pwl1->AddLineToPoint(Point2D(8.0, 3.0));
    pwl1->AddLineToPoint(Point2D(20.0, 3.0));
    pg2d->AddSubPath(pwl1);*/
    
    //Arc2D* arc1 = new Arc2D(Point2D(2.0, -3.0), 3.0, SIMD_HALF_PI, -SIMD_HALF_PI);
    //pg2d->AddSubPath(arc1);
    
    Bezier2D* bez1 = new Bezier2D(Point2D(0.0, 0.0), Point2D(8, -3.0), Point2D(4.0, 0), Point2D(4.0,-3.0), false);
    pg2d->AddSubPath(bez1, true);
    
    AcroverPathFollowingController* pathFollowing = new AcroverPathFollowingController("PathFollowing", pg2d, traj, enc1, longitudinal, lateral, 100.0);
    
    //Order is important!!!
    AddController(pathFollowing);
    AddController(longitudinal);
    AddController(lateral);
    AddController(drive1);
    AddController(drive2);
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 0.5), 2.0, btVector3(0,0,1.0), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 100.f, true);
    trackb->Activate();
    trackb->GlueToEntity(cyl);
    trackb->Rotate(btQuaternion(-M_PI_2, 0.0, 0.0));
    AddView(trackb);
}