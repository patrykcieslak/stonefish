//
//  Odometry.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/11/2017.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "Odometry.h"

Odometry::Odometry(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    channels.push_back(SensorChannel("Position X", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Position Y", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Position Z", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Orientation X", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Orientation Y", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Orientation Z", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Orientation W", QUANTITY_UNITLESS));
    channels.push_back(SensorChannel("Angular velocity X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QUANTITY_ANGULAR_VELOCITY));
}

void Odometry::InternalUpdate(btScalar dt)
{
    //Calculate transformation from global to imu frame
    btTransform odomTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    btVector3 pos = odomTrans.getOrigin();
    btVector3 v = odomTrans.getBasis().inverse() * attach->getLinearVelocity();
    btQuaternion orn = odomTrans.getRotation();
    btVector3 av = odomTrans.getBasis().inverse() * attach->getAngularVelocity();
    
    //Save sample
    btScalar values[13] = {pos.x(), pos.y(), pos.z(), v.x(), v.y(), v.z(), orn.x(), orn.y(), orn.z(), orn.w(), av.x(), av.y(), av.z()};
    Sample s(13, values);
    AddSampleToHistory(s);
}
   
void Odometry::SetNoise(btScalar positionStdDev, btScalar velocityStdDev, btScalar orientationStdDev, btScalar angularVelocityStdDev)
{
    channels[0].setStdDev(positionStdDev);
    channels[1].setStdDev(positionStdDev);
    channels[2].setStdDev(positionStdDev);
}

btTransform Odometry::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}