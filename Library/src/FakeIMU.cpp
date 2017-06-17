//
//  FakeIMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FakeIMU.h"
#include "SimulationApp.h"

FakeIMU::FakeIMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency, unsigned int historyLength) : IMU(uniqueName, attachment, relFrame, frequency, historyLength)
{
}

void FakeIMU::Reset()
{
    //calculate transformation from global to imu frame
    btMatrix3x3 toImuFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //get velocity
    lastV = solid->getLinearVelocityInLocalPoint(relToSolid.getOrigin());
    lastV = toImuFrame * lastV;
    
    SimpleSensor::Reset();
}

void FakeIMU::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to imu frame
    btMatrix3x3 toImuFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //get acceleration
    //inertial component
    btVector3 actualV = solid->getLinearVelocityInLocalPoint(relToSolid.getOrigin());
    actualV = toImuFrame * actualV;
    btVector3 acc = - (actualV - lastV)/dt;
    lastV = actualV;
    
    //gravity component
    btVector3 grav = SimulationApp::getApp()->getSimulationManager()->getGravity();
    grav = toImuFrame * grav;
    acc += grav;
    
    //get angular velocity
    btVector3 av = solid->getAngularVelocity();
    av = toImuFrame * av;
    
    //get angles
    btScalar yaw, pitch, roll;
    btMatrix3x3 globalFrame = solid->getTransform().getBasis() * relToSolid.getBasis();
    globalFrame.getEulerYPR(yaw, pitch, roll);
    
    //save sample
    btScalar values[9] = {roll, pitch, yaw, av.x(), av.y(), av.z(), acc.x(), acc.y(), acc.z()};
    Sample s(9, values);
    AddSampleToHistory(s);
}