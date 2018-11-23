//
//  ForceTorque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/ForceTorque.h"

#include "utils/MathUtil.hpp"

using namespace sf;

ForceTorque::ForceTorque(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    attach = attachment;
    g2s = geomToSensor;
    lastFrame = btTransform::getIdentity();
    
    channels.push_back(SensorChannel("Force X", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Y", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Z", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Torque X", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Y", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Z", QUANTITY_TORQUE));
}

ForceTorque::ForceTorque(std::string uniqueName, const btTransform& geomToSensor, btScalar frequency, int historyLength) :
    ForceTorque(uniqueName, NULL, geomToSensor, frequency, historyLength)
{
}

void ForceTorque::InternalUpdate(btScalar dt)
{
    if(j != NULL)
    {
        btVector3 force(j->getFeedback(0), j->getFeedback(1), j->getFeedback(2));
        btVector3 torque(j->getFeedback(3), j->getFeedback(4), j->getFeedback(5));
    
        if(j->isMultibodyJoint())
        {  
            force /= dt;
            torque /= dt;
        }
    
        lastFrame = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
        btMatrix3x3 toSensor = lastFrame.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
	
        btScalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
        Sample s(6, values);
        AddSampleToHistory(s);
    }
    else
    {   
        btVector3 force, torque;
        unsigned int childId = fe->getJointFeedback(jId, force, torque);
        lastFrame = fe->getLink(childId).solid->getGeomToCOGTransform().inverse() * g2s;
        btMatrix3x3 toSensor = lastFrame.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
        lastFrame = fe->getLink(childId).solid->getTransform() * lastFrame; //From local to global
        
        btScalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
        Sample s(6, values);
        AddSampleToHistory(s);
    }
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
    
    Renderable item;
    item.type = RenderableType::SENSOR_CS;
    item.model = glMatrixFromBtTransform(lastFrame);
    items.push_back(item);
    
    return items;
}
