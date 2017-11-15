//
//  Current.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Current.h"

Current::Current(std::string uniqueName, DCMotor* m, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, btTransform::getIdentity(), frequency, historyLength)
{
    motor = m;
    channels.push_back(SensorChannel("Current", QUANTITY_CURRENT));
}

void Current::Reset()
{
    SimpleSensor::Reset();
}

void Current::InternalUpdate(btScalar dt)
{
    //save sample
    btScalar m = motor->getCurrent();
    Sample s(1, &m);
    AddSampleToHistory(s);
}