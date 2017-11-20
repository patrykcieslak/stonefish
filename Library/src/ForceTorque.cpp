//
//  ForceTorque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "ForceTorque.h"
#include "MathsUtil.hpp"

ForceTorque::ForceTorque(std::string uniqueName, Joint* j, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    joint = j;
    attach = attachment;
    
    channels.push_back(SensorChannel("Force X", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Y", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Z", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Torque X", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Y", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Z", QUANTITY_TORQUE));
}

void ForceTorque::Reset()
{
    SimpleSensor::Reset();
}

void ForceTorque::InternalUpdate(btScalar dt)
{
    btVector3 force(joint->getFeedback(0), joint->getFeedback(1), joint->getFeedback(2));
    btVector3 torque(joint->getFeedback(3), joint->getFeedback(4), joint->getFeedback(5));
    
    if(joint->isMultibodyJoint())
    {
        force /= dt;
        torque /= dt;
    }
    
    btTransform ftTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    force = ftTrans.getBasis().inverse() * force;
    torque = ftTrans.getBasis().inverse() * torque;
	
    btScalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
    Sample s(6, values);
    AddSampleToHistory(s);
}

void ForceTorque::SetRange(const btVector3& forceMax, const btVector3& torqueMax)
{
    channels[0].rangeMin = -forceMax.getX();
    channels[1].rangeMin = -forceMax.getY();
    channels[2].rangeMin = -forceMax.getZ();
    channels[0].rangeMax = forceMax.getX();
    channels[1].rangeMax = forceMax.getY();
    channels[2].rangeMax = forceMax.getZ();
    
    channels[3].rangeMin = -torqueMax.getX();
    channels[4].rangeMin = -torqueMax.getY();
    channels[5].rangeMin = -torqueMax.getZ();
    channels[3].rangeMax = torqueMax.getX();
    channels[4].rangeMax = torqueMax.getY();
    channels[5].rangeMax = torqueMax.getZ();
}
    
void ForceTorque::SetNoise(btScalar forceStdDev, btScalar torqueStdDev)
{
    channels[0].setStdDev(forceStdDev);
    channels[1].setStdDev(forceStdDev);
    channels[2].setStdDev(forceStdDev);
    channels[3].setStdDev(torqueStdDev);
    channels[4].setStdDev(torqueStdDev);
    channels[5].setStdDev(torqueStdDev);
}
    
std::vector<Renderable> ForceTorque::Render()
{
    std::vector<Renderable> items(0);
    
    btTransform ftTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    Renderable item;
    item.type = RenderableType::SENSOR_CS;
    item.model = glMatrixFromBtTransform(ftTrans);
    items.push_back(item);

    return items;
}