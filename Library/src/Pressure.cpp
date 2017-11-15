//
//  Pressure.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Pressure.h"
#include "SimulationApp.h"

Pressure::Pressure(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    channels.push_back(SensorChannel("Pressure", QUANTITY_PRESSURE));
}

void Pressure::InternalUpdate(btScalar dt)
{
    btTransform pressureTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    btScalar data(0.); //Gauge pressure //data(101325.); //Pa (1 atm)
    
    Liquid* liq = SimulationApp::getApp()->getSimulationManager()->getLiquid();
    if(liq != NULL)
        data += liq->GetPressure(pressureTrans.getOrigin());
    
    //Record sample
    Sample s(1, &data);
    AddSampleToHistory(s);
}

void Pressure::SetRange(btScalar max)
{
    channels[0].rangeMin = btScalar(0);
    channels[0].rangeMax = max;
}
    
void Pressure::SetNoise(btScalar pressureStdDev)
{
    channels[0].setStdDev(pressureStdDev);
}

btTransform Pressure::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}