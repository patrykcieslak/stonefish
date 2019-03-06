//
//  ForceTorque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/ForceTorque.h"

#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "sensors/Sample.h"
#include "joints/Joint.h"

namespace sf
{

ForceTorque::ForceTorque(std::string uniqueName, SolidEntity* attachment, const Transform& origin, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    attach = attachment;
    o2s = origin;
    lastFrame = Transform::getIdentity();
    
    channels.push_back(SensorChannel("Force X", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Y", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Force Z", QUANTITY_FORCE));
    channels.push_back(SensorChannel("Torque X", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Y", QUANTITY_TORQUE));
    channels.push_back(SensorChannel("Torque Z", QUANTITY_TORQUE));
}

ForceTorque::ForceTorque(std::string uniqueName, const Transform& origin, Scalar frequency, int historyLength) :
    ForceTorque(uniqueName, NULL, origin, frequency, historyLength)
{
}

Transform ForceTorque::getSensorFrame()
{
    return lastFrame;
}

void ForceTorque::InternalUpdate(Scalar dt)
{
    if(j != NULL && attach != NULL)
    {
        Vector3 force(j->getFeedback(0), j->getFeedback(1), j->getFeedback(2));
        Vector3 torque(j->getFeedback(3), j->getFeedback(4), j->getFeedback(5));
    
        if(j->isMultibodyJoint())
        {  
            force /= dt;
            torque /= dt;
        }
    
        lastFrame = attach->getOTransform() * o2s;
        Matrix3 toSensor = lastFrame.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
	
        Scalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
        Sample s(6, values);
        AddSampleToHistory(s);
    }
    else
    {   
        Vector3 force, torque;
        unsigned int childId = fe->getJointFeedback(jId, force, torque);
        lastFrame = fe->getLink(childId).solid->getCG2OTransform() * o2s;
        Matrix3 toSensor = lastFrame.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
        lastFrame = fe->getLink(childId).solid->getCGTransform() * lastFrame; //From local to global
        
        Scalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
        Sample s(6, values);
        AddSampleToHistory(s);
    }
}

void ForceTorque::setRange(const Vector3& forceMax, const Vector3& torqueMax)
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
    
void ForceTorque::setNoise(Scalar forceStdDev, Scalar torqueStdDev)
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
    item.model = glMatrixFromTransform(lastFrame);
    items.push_back(item);
    
    return items;
}

}
