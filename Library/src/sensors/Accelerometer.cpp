//
//  Accelerometer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "sensors/Accelerometer.h"

Accelerometer::Accelerometer(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    channels.push_back(SensorChannel("Acceleration X", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Acceleration Y", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Acceleration Z", QUANTITY_ACCELERATION));
    channels.push_back(SensorChannel("Angular acceleration X", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular acceleration Y", QUANTITY_ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular acceleration Z", QUANTITY_ANGULAR_VELOCITY));
}

void Accelerometer::InternalUpdate(btScalar dt)
{
    //Calculate transformation from global to imu frame
    btTransform accTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
	//Get acceleration
	btVector3 la = accTrans.getBasis().inverse() * (attach->getLinearAcceleration() + attach->getAngularAcceleration().cross(accTrans.getOrigin() - attach->getTransform().getOrigin()));
	
    //Get angular acceleration
    btVector3 aa = accTrans.getBasis().inverse() * attach->getAngularAcceleration();
    
    //Save sample
    btScalar values[6] = {la.x(), la.y(), la.z(), aa.x(), aa.y(), aa.z()};
    Sample s(6, values);
    AddSampleToHistory(s);
}

void Accelerometer::SetRange(btScalar linearAccMax, btScalar angularAccMax)
{
	channels[0].rangeMin = -linearAccMax;
    channels[1].rangeMin = -linearAccMax;
    channels[2].rangeMin = -linearAccMax;
    channels[0].rangeMax = linearAccMax;
    channels[1].rangeMax = linearAccMax;
    channels[2].rangeMax = linearAccMax;
	
    channels[3].rangeMin = -angularAccMax;
    channels[4].rangeMin = -angularAccMax;
    channels[5].rangeMin = -angularAccMax;
    channels[3].rangeMax = angularAccMax;
    channels[4].rangeMax = angularAccMax;
    channels[5].rangeMax = angularAccMax;
}
    
void Accelerometer::SetNoise(btScalar linearAccStdDev, btScalar angularAccStdDev)
{
    channels[0].setStdDev(linearAccStdDev);
    channels[1].setStdDev(linearAccStdDev);
    channels[2].setStdDev(linearAccStdDev);
    channels[3].setStdDev(angularAccStdDev);
    channels[4].setStdDev(angularAccStdDev);
    channels[5].setStdDev(angularAccStdDev);
}

btTransform Accelerometer::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}
