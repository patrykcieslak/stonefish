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
#include "FakeRotaryEncoder.h"
#include "Current.h"
#include "SystemUtil.h"
#include "DCMotor.h"
#include "Trajectory.h"
#include "PathGenerator2D.h"
#include "MISOStateSpaceController.h"

RollingTestManager::RollingTestManager(btScalar stepsPerSecond) : SimulationManager(MMKS, true, stepsPerSecond, DANTZIG, INCLUSIVE)
{
}

void RollingTestManager::BuildScenario()
{
    //--------------------Using MMSK unit system--------------------
    setICSolverParams(true);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MMKS, 4.0), 0.5);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MMKS, 2.0), 0.2);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 0.8, 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Concrete", 0.9, 0.8);
    getMaterialManager()->SetMaterialsInteraction("Rubber", "Rubber", 0.8, 0.7);
    
    ///////LOOKS///////////
    char path[1024];
    GetCWD(path, 1024);
    GetDataPath(path, 1024-32);
    strcat(path, "grid.png");
    
    Look grid = CreateOpaqueLook(glm::vec3(1.f, 1.f, 1.f), 0.5f, 0.8f, 1.7f, path);
    Look grey = CreateOpaqueLook(glm::vec3(0.7f, 0.7f, 0.7f), 0.5f, 0.5f, 1.3f);
    Look shiny = CreateOpaqueLook(glm::vec3(0.1f, 0.1f, 0.1f), 0.3f, 0.1f, 1.2f);
    
    /////////////OBJECTS//////////////
    PlaneEntity* floor = new PlaneEntity("Floor", 2000000.f, getMaterialManager()->getMaterial("Concrete"), grid, btTransform(btQuaternion(0,0,0), btVector3(0,0,50.f)));
    floor->setRenderable(true);
    AddEntity(floor);
    
    TorusEntity* torus = new TorusEntity("Wheel", 200.f, 20.f, getMaterialManager()->getMaterial("Rubber"), shiny);
    AddSolidEntity(torus, btTransform(btQuaternion(0.f,0.f,0.0f), btVector3(0,0,300.f)));
    
    CylinderEntity* cyl = new CylinderEntity("Cart", 170.f, 20.f, getMaterialManager()->getMaterial("Concrete"), grey);
    AddSolidEntity(cyl, btTransform(btQuaternion(0,0,0), btVector3(0,0, 300.f)));
    cyl->SetArbitraryPhysicalProperties(1.5f, btVector3(1500,1500,1500), btTransform(btQuaternion::getIdentity(), btVector3(0,0.0f,-50.f)));
    
    BoxEntity* box = new BoxEntity("Lever", btVector3(20.f,20.f,150.f), getMaterialManager()->getMaterial("Concrete"), grey);
    AddSolidEntity(box, btTransform(btQuaternion::getIdentity(), btVector3(0.f, 0.f, 300.f-150.f/2.f)));
    
    RevoluteJoint* revoCW = new RevoluteJoint("CartWheel", torus, cyl, btVector3(0,0,300.f), btVector3(0,1.f,0), false);
    AddJoint(revoCW);
    revoCW->setDamping(0.001, 0.2);
    
    DCMotor* motorCW = new DCMotor("DCXWheel", revoCW, 0.212f, 0.0774e-3, 1.f/408.f, 23.4e-3, 0.0000055f);
    AddActuator(motorCW);
    motorCW->SetupGearbox(true, 10.0, 1.0);
    //motorCW->setVoltage(0.2);
    
    RevoluteJoint* revoCL = new RevoluteJoint("CartLever", box, cyl, btVector3(0.f,0.f,300.f), btVector3(1.f,0,0), false);
    AddJoint(revoCL);
    
    DCMotor* motorCL = new DCMotor("DCXLever", revoCL, 0.212f, 0.0774e-3, 1.f/408.f, 23.4e-3, 0.0000055f);
    AddActuator(motorCL);
    
    Contact* c = AddContact(torus, floor, 0);
    c->setDisplayMask(CONTACT_DISPLAY_PATH_A);
    
    /////////////SENSORS////////////////
    FakeIMU* imu = new FakeIMU("IMU", cyl, btTransform::getIdentity(), -1, 10000);
    AddSensor(imu);
    //0 -> Theta (Roll)
    //1 -> Psi (Pitch)
    //2 -> Phi (Yaw)
    
    FakeRotaryEncoder* encCW = new FakeRotaryEncoder("EncoderWheel", revoCW, -1, 10000);
    AddSensor(encCW);
    
    Current* curCW = new Current("CurrentWheel", motorCW, -1, 10000);
    AddSensor(curCW);
    
    Trajectory* traj = new Trajectory("Trajectory", torus, btVector3(0,0,100), btScalar(0.), 10000);
    traj->setRenderable(true);
    AddSensor(traj);
    
    //////////////CONTROL///////////////
    //Longitudinal stabilization
    Mux* longSensors = new Mux();
    longSensors->AddComponent(encCW, 0); //Gamma
    longSensors->AddComponent(imu, 1);   //Psi
    longSensors->AddComponent(encCW, 1); //dGamma
    longSensors->AddComponent(imu, 4);   //dPsi
    longSensors->AddComponent(curCW, 0); //Current
    
    std::vector<btScalar> gains;
    gains.push_back(0.0);
    gains.push_back(100.0);
    gains.push_back(3.0);
    gains.push_back(1.0);
    gains.push_back(0.0);
    
    std::vector<btScalar> desired;
    desired.push_back(0.0);
    desired.push_back(0.0);
    desired.push_back(UnitSystem::Angle(true, 90.0));
    desired.push_back(0.0);
    desired.push_back(0.0);
    
    MISOStateSpaceController* longitudinal = new MISOStateSpaceController("Longitudinal", longSensors, motorCW, 5.0, 200.0);
    longitudinal->SetGains(gains);
    longitudinal->SetDesiredValues(desired);
    AddController(longitudinal);
    
    //Lateral stabilization
    
    
    
    //Path generation
    PathGenerator2D* pg2d = new PathGenerator2D(PLANE_XY);
    pg2d->setRenderable(true);
    AddPathGenerator(pg2d);
    
    Pwl2D* pwl1 = new Pwl2D(Point2D(1000.0, 1000.0));
    pwl1->AddLineToPoint(Point2D(2000.0, 1000.0));
    pwl1->AddLineToPoint(Point2D(3000.0, 2000.0));
    pg2d->AddSubPath(pwl1);
    
    Arc2D* arc1 = new Arc2D(Point2D(4000.0, 2500.0), 1000.0, 0, M_PI * 0.8);
    pg2d->AddSubPath(arc1);
    
    Bezier2D* bez1 = new Bezier2D(Point2D(4000.0, 1000.0), Point2D(2000.0, -4000.0), Point2D(5000.0, 0.0), Point2D(1000.0, - 500.0), false);
    pg2d->AddSubPath(bez1, false);
    
    //Path following
    
    
    
    
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 500.0), 2000.0, btVector3(0,0,1.0), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 100000.f, false);
    trackb->Activate();
    trackb->GlueToEntity(cyl);
    trackb->Rotate(btQuaternion(M_PI * 1.1, 0.0, 0.0));
    AddView(trackb);
}