//
//  Current.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Current.h"

Current::Current(std::string uniqueName, DCMotor* m, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    motor = m;
}

void Current::Reset()
{
    Sensor::Reset();
}

void Current::InternalUpdate(btScalar dt)
{
    //save sample
    btScalar ext = motor->getCurrent();
    Sample s(1, &ext);
    AddSampleToHistory(s);
}

unsigned short Current::getNumOfDimensions()
{
    return 1;
}