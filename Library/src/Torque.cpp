//
//  Torque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "Torque.h"
#include "MathsUtil.hpp"

Torque::Torque(std::string uniqueName, FeatherstoneEntity* f, unsigned int jointId, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    fe = f;
    jId = jointId; 
    
    channels.push_back(SensorChannel("Torque", QUANTITY_TORQUE));
}

void Torque::Reset()
{
    SimpleSensor::Reset();
}

void Torque::InternalUpdate(btScalar dt)
{
    btVector3 force, torque;
    unsigned int childId = fe->getJointFeedback(jId, force, torque);
    btVector3 axis = fe->getJointAxis(jId);
    //btScalar tau = torque.dot(axis); //
    btScalar tau = fe->getMotorImpulse(jId)/dt;
    
    btScalar values[1] = {tau};
    Sample s(1, values);
    AddSampleToHistory(s);
}

void Torque::SetRange(btScalar max)
{
    channels[0].rangeMin = -max;
    channels[0].rangeMax = max;
}

void Torque::SetNoise(btScalar stdDev)
{
    channels[0].setStdDev(stdDev);
}

std::vector<Renderable> Torque::Render()
{
    std::vector<Renderable> items(0);
    return items;
}