//
//  FOG.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/FOG.h"

using namespace sf;

FOG::FOG(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Heading", QUANTITY_ANGLE));
}

void FOG::InternalUpdate(Scalar dt)
{
    //get angles
    Scalar yaw, pitch, roll;
    getSensorFrame().getBasis().getEulerYPR(yaw, pitch, roll);
    
    //record sample
    Sample s(1, &yaw);
    AddSampleToHistory(s);
}

void FOG::SetNoise(Scalar headingStdDev)
{
    channels[0].setStdDev(headingStdDev);
}
