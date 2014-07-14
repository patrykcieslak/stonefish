//
//  Current.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Current.h"

#pragma mark Constructors
Current::Current(std::string uniqueName, DCMotor* m, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    motor = m;
    channels.push_back(SensorChannel("Current", QUANTITY_CURRENT));
}

#pragma mark - Methods
void Current::Reset()
{
    Sensor::Reset();
}

void Current::InternalUpdate(btScalar dt)
{
    //save sample
    btScalar m = motor->getCurrent();
    Sample s(1, &m);
    AddSampleToHistory(s);
}