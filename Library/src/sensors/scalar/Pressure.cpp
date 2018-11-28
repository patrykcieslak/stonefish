//
//  Pressure.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Pressure.h"

#include "core/SimulationApp.h"

using namespace sf;

Pressure::Pressure(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Pressure", QUANTITY_PRESSURE));
}

void Pressure::InternalUpdate(Scalar dt)
{
    Scalar data(0.); //Gauge pressure //data(101325.); //Pa (1 atm)
    
    Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(liq != NULL)
        data += liq->GetPressure(getSensorFrame().getOrigin());
    
    //Record sample
    Sample s(1, &data);
    AddSampleToHistory(s);
}

void Pressure::SetRange(Scalar max)
{
    channels[0].rangeMin = Scalar(0);
    channels[0].rangeMax = max;
}
    
void Pressure::SetNoise(Scalar pressureStdDev)
{
    channels[0].setStdDev(pressureStdDev);
}
