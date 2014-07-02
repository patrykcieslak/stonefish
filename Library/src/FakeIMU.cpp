//
//  FakeIMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FakeIMU.h"

FakeIMU::FakeIMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = UnitSystem::SetTransform(relFrame);

    Reset();
}

void FakeIMU::Reset()
{
    Sensor::Reset();
    lastV = btVector3(0.,0.,0.);
}

void FakeIMU::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to imu frame
    //btMatrix3x3 toImuFrame = relToSolid.getBasis().inverse() * solid->getRigidBody()->getCenterOfMassTransform().getBasis().inverse();
    btMatrix3x3 toImuFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //get acceleration
    //inertial component
    //btVector3 actualV = solid->getRigidBody()->getVelocityInLocalPoint(relToSolid.getOrigin()); //get velocity in sensor location
    btVector3 actualV = solid->getLinearVelocityInLocalPoint(relToSolid.getOrigin());
    actualV = toImuFrame * actualV;
    btVector3 acc = -(actualV - lastV)/dt;
    lastV = actualV;
    
    //gravity component
    //btVector3 grav = solid->getRigidBody()->getGravity();
    btVector3 grav = btVector3(0,0,-9.81);
    grav = toImuFrame * grav;
    acc += grav;
    
    //get angular velocity
    //btVector3 av = solid->getRigidBody()->getAngularVelocity();
    btVector3 av = solid->getAngularVelocity();
    av = toImuFrame * av;
    
    //get angles
    btScalar yaw, pitch, roll;
    toImuFrame.getEulerZYX(yaw, pitch, roll);
    
    //select axis and convert to external unit system or G's
    roll = UnitSystem::GetAngle(roll);
    pitch = UnitSystem::GetAngle(pitch);
    yaw = UnitSystem::GetAngle(yaw);
    acc = UnitSystem::GetAcceleration(acc);
    av = UnitSystem::GetAngularVelocity(av);
    
    //save sample
    btScalar values[9] = {roll, pitch, yaw, av.x(), av.y(), av.z(), acc.x(), acc.y(), acc.z()};
    Sample s(9, values);
    AddSampleToHistory(s);
}

unsigned short FakeIMU::getNumOfDimensions()
{
    return 9; //acceleration, angular velocity, tilt
}
