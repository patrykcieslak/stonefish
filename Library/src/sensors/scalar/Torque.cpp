//
//  Torque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Torque.h"

#include "utils/MathUtil.hpp"

using namespace sf;

Torque::Torque(std::string uniqueName, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Torque", QUANTITY_TORQUE));
}

void Torque::InternalUpdate(Scalar dt)
{
    //Vector3 force, torque;
    //fe->getJointFeedback(jId, force, torque);
    //Vector3 axis = fe->getJointAxis(jId);
    //Scalar tau = torque.dot(axis);
    if(fe != NULL)
    {
        Scalar tau = fe->getMotorImpulse(jId)/dt;
    
        Scalar values[1] = {tau};
        Sample s(1, values);
        AddSampleToHistory(s);
    }
}

void Torque::SetRange(Scalar max)
{
    channels[0].rangeMin = -max;
    channels[0].rangeMax = max;
}

void Torque::SetNoise(Scalar stdDev)
{
    channels[0].setStdDev(stdDev);
}
