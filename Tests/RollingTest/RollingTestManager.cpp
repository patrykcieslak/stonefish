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
#include "DCMotor.h"
#include "Trajectory.h"
#include "PathGenerator2D.h"
#include "FeatherstoneEntity.h"
#include "MISOStateSpaceController.h"
#include "MonoWheelLateral.h"
#include "SignalGenerator.h"
#include "MonoWheelPathFollowing.h"

RollingTestManager::RollingTestManager(btScalar stepsPerSecond) : SimulationManager(MKS, true, stepsPerSecond, DANTZIG, INCLUSIVE)
{
}

void RollingTestManager::BuildScenario()
{
    //--------------------Using MMSK unit system--------------------
    setICSolverParams(true);
    OpenGLPipeline::getInstance()->setDebugSimulation(false);
    OpenGLPipeline::getInstance()->setVisibleHelpers(false, false, false, false, false, false, false);
    
    ///////MATERIALS////////
    getMaterialManager()->CreateMaterial("Concrete", UnitSystem::Density(CGS, MKS, 4.0), 0.2);
    getMaterialManager()->CreateMaterial("Rubber", UnitSystem::Density(CGS, MKS, 2.0), 0.5);
    getMaterialManager()->SetMaterialsInteraction("Concrete", "Rubber", 1.0, 1.0);
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
    
    /*TorusEntity* torus = new TorusEntity("Wheel", 200.f, 20.f, getMaterialManager()->getMaterial("Rubber"), shiny);
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
    AddSensor(traj);*/
    
    //Wheel
    //SphereEntity* torus = new SphereEntity("Wheel", 0.02, getMaterialManager()->getMaterial("Rubber"), shiny);
    btScalar Rt = 0.205;
    btScalar rt = 0.015875;
    TorusEntity* torus = new TorusEntity("Wheel", Rt - rt, rt, getMaterialManager()->getMaterial("Rubber"), shiny);
    torus->SetArbitraryPhysicalProperties(0.5, btVector3(0.05, 0.05, 0.05), btTransform::getIdentity());
    
    //Cart
    CylinderEntity* cyl = new CylinderEntity("Cart", 0.17, 0.01, getMaterialManager()->getMaterial("Concrete"), grey);
    cyl->SetArbitraryPhysicalProperties(1.5, btVector3(0.05, 0.1, 0.05), btTransform(btQuaternion::getIdentity(), btVector3(0.0, 0.0, -0.055)));
    
    //Lever
    BoxEntity* box = new BoxEntity("Lever", btVector3(0.02, 0.03, 0.12), getMaterialManager()->getMaterial("Concrete"), orange);
    box->SetArbitraryPhysicalProperties(0.5, btVector3(0.0001, 0.0001, 0.000001), btTransform(btQuaternion::getIdentity(), btVector3(0,0,-0.06)));
    
    //Robot
    FeatherstoneEntity* mwr = new FeatherstoneEntity("Mono-wheel robot", 3, torus, btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0.3)), getDynamicsWorld(), false);
    mwr->setBaseRenderable(true);
    mwr->AddLink(cyl, btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0.3)), getDynamicsWorld());
    mwr->AddRevoluteJoint(0, 1, btVector3(0, 0, 0.3), btVector3(0,1,0));
    mwr->AddLink(box, btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0.245 - 0.06)), getDynamicsWorld());
    mwr->AddRevoluteJoint(1, 2, btVector3(0, 0, 0.245), btVector3(1,0,0));
    mwr->setJointIC(1, 0.0, 0.0);
    AddEntity(mwr);
    
    Contact* c = AddContact(torus, floor, 100000);
    c->setDisplayMask(CONTACT_DISPLAY_PATH_A);
    
    unsigned int sensorHistory = 5000;
    
    DCMotor* motorCW = new DCMotor("DCXWheel", mwr, 0, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    motorCW->SetupGearbox(true, 10.0, 1.0);
    AddActuator(motorCW);
    
    DCMotor* motorCL = new DCMotor("DCXLever", mwr, 1, 0.212, 0.0774e-3, 1.0/408.0, 23.4e-3, 0.0000055);
    motorCL->SetupGearbox(true, 2.0, 1.0);
    AddActuator(motorCL);

    FakeIMU* imu = new FakeIMU("IMU", cyl, btTransform::getIdentity(), -1, sensorHistory);
    AddSensor(imu);
    //0 -> Theta (Roll)
    //1 -> Psi (Pitch)
    //2 -> Phi (Yaw)
    
    FakeRotaryEncoder* encCW = new FakeRotaryEncoder("EncoderWheel", mwr, 0, -1, sensorHistory);
    AddSensor(encCW);
    
    FakeRotaryEncoder* encCL = new FakeRotaryEncoder("EncoderLever", mwr, 1, -1, sensorHistory);
    AddSensor(encCL);
    
    Current* curCW = new Current("CurrentWheel", motorCW, -1, sensorHistory);
    AddSensor(curCW);

    Current* curCL = new Current("CurrentWheel", motorCL, -1, sensorHistory);
    AddSensor(curCL);

    Trajectory* traj = new Trajectory("Trajectory", cyl, btTransform(btQuaternion::getIdentity(), btVector3(0,0,0)), -1, sensorHistory);
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
    gains.push_back(30.0);
    gains.push_back(10.0);
    gains.push_back(20.0);
    gains.push_back(-0.01);
    
    //0.0  2.6337e+03   3.3972e+00   1.8717e+02  -1.1620e-01
    //0.0  4.4261e+03   3.3675e+01   1.2448e+01  -1.1244e-01
    /*gains.push_back(0.0);
    gains.push_back(4.4261e+03);
    gains.push_back(3.3675e+01);
    gains.push_back(1.2448e+01);
    gains.push_back(-1.1244e-01);*/
    
    MISOStateSpaceController* longitudinal = new MISOStateSpaceController("Longitudinal", longSensors, motorCW, 12.0, 300.0);
    longitudinal->SetGains(gains);
    
    MonoWheelLateral* lateral = new MonoWheelLateral("Lateral", imu, encCL, encCW, curCL, motorCL, 12.0, 300.0);
    
    ////////////PRESCRIBED MOTION
    //Longitudinal
    SignalGenerator* forwardSg = new SignalGenerator();
    PwlSignal* forwardPwlSig = new PwlSignal(0.);
    
    //forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -180.0), 1.0);
    //forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -180.0), 50.0);
    
    /*forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -360.0), 3.0);
    forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -360.0), 12.0);
    forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -180.0), 14.0);
    forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -180.0), 50.0);
    forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -260.0), 58.0);
    forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -170.0), 65.0);
    forwardPwlSig->AddValueAtTime(UnitSystem::AngularVelocity(true, -180.0), 14.0);
    forwardPwlSig->AddValueAtTime(0., 72.0);*/
    forwardSg->AddComponent(forwardPwlSig);
    
    longitudinal->setReferenceSignalGenerator(2, forwardSg);
    AddController(longitudinal);
    
    //Lateral
    SignalGenerator* lateralSg = new SignalGenerator();
    PwlSignal* lateralPwlSig = new PwlSignal(0.);
    
    lateralPwlSig->AddValueAtTime(0., 2.0);
    lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, 2.0), 2.0);
    
    //lateralPwlSig->AddValueAtTime(0., 2.0);
    //lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, 8.0), 10.0);
    //lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, 8.0), 50.0);
    
    /*lateralPwlSig->AddValueAtTime(0., 4.0);
    lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, 2.0), 4.0);
    lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, 2.0), 20.0);
    lateralPwlSig->AddValueAtTime(0., 20.0);
    lateralPwlSig->AddValueAtTime(0., 40.0);
    lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, -2.0), 40.0);
    lateralPwlSig->AddValueAtTime(UnitSystem::Angle(true, -2.0), 65.0);
    lateralPwlSig->AddValueAtTime(0., 70.0);*/
    lateralSg->AddComponent(lateralPwlSig);
    
    lateral->setReferenceSignalGenerator(0, lateralSg);
    AddController(lateral);
    
    ////////////PRESCRIBED PATH
    /*PathGenerator2D* pg2d = new PathGenerator2D(PLANE_XY);
    pg2d->setRenderable(true);
    
    Pwl2D* pwl1 = new Pwl2D(Point2D(0.0, 0.0));
    pwl1->AddLineToPoint(Point2D(2.0, 0.0));
    pwl1->AddLineToPoint(Point2D(10.0, 1.0));
    pg2d->AddSubPath(pwl1);
    
    //Arc2D* arc1 = new Arc2D(Point2D(0.0, -8.0), 8.0, SIMD_HALF_PI, -SIMD_HALF_PI);
    //pg2d->AddSubPath(arc1);
    
    //Bezier2D* bez1 = new Bezier2D(Point2D(4.0, 1.0), Point2D(2.0, -4.0), Point2D(5.0, 0.0), Point2D(1.0, -0.5), false);
    //pg2d->AddSubPath(bez1, false);
    
    MonoWheelPathFollowing* pathFollowing = new MonoWheelPathFollowing("PathFollowing", pg2d, traj, encCW, lateral, longitudinal, 100.0);
    
    //Order is important!!!
    AddController(pathFollowing);
    AddController(lateral);
    AddController(longitudinal);
   */
    //////CAMERA & LIGHT//////
    OpenGLTrackball* trackb = new OpenGLTrackball(btVector3(0, 0, 0.5), 2.0, btVector3(0,0,1.0), 0, 0, SimulationApp::getApp()->getWindowWidth(), SimulationApp::getApp()->getWindowHeight(), 60.f, 100.f, true);
    trackb->Activate();
    trackb->GlueToEntity(cyl);
    trackb->Rotate(btQuaternion(-M_PI_2, 0.0, 0.0));
    AddView(trackb);
}