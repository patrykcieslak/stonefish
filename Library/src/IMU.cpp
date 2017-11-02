//
//  IMU.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "IMU.h"

IMU::IMU(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, frequency, historyLength)
{
    attach = attachment;
    g2s = UnitSystem::SetTransform(geomToSensor);
    channels.push_back(SensorChannel("Roll", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Pitch", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Yaw", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QUANTITY_ANGULAR_VELOCITY));
}

void IMU::InternalUpdate(btScalar dt)
{
    //Calculate transformation from global to imu frame
    btTransform imuTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    //Get angular velocity
    btVector3 av = imuTrans.getBasis().inverse() * attach->getAngularVelocity();
    
    //Get angles
    btScalar yaw, pitch, roll;
    imuTrans.getBasis().getEulerYPR(yaw, pitch, roll);
    
    //Save sample
    btScalar values[6] = {roll, pitch, yaw, av.x(), av.y(), av.z()};
    Sample s(6, values);
    AddSampleToHistory(s);
}