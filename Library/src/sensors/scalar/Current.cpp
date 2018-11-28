//
//  Current.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Current.h"

using namespace sf;

Current::Current(std::string uniqueName, Scalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    motor = NULL;
    channels.push_back(SensorChannel("Current", QUANTITY_CURRENT));
}

void Current::AttachToMotor(DCMotor* m)
{
    if(m != NULL)
        motor = m;
}

void Current::InternalUpdate(Scalar dt)
{
    //read current
    Scalar current = Scalar(0);
    if(motor != NULL)
        current = motor->getCurrent();
    
    //record sample
    Sample s(1, &current);
    AddSampleToHistory(s);
}

SensorType Current::getType()
{
    return SensorType::SENSOR_OTHER;
}

