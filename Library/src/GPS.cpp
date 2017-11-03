//
//  GPS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "GPS.h"
#include "SimulationApp.h"

GPS::GPS(std::string uniqueName, btScalar latitude, btScalar longitude, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, frequency, historyLength)
{
    attach = attachment;
    g2s = UnitSystem::SetTransform(geomToSensor);
    homeLatitude = UnitSystem::SetAngle(latitude);
    homeLongitude = UnitSystem::SetAngle(longitude);
    channels.push_back(SensorChannel("Latitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Longitude", QUANTITY_ANGLE));
}

void GPS::InternalUpdate(btScalar dt)
{
    btTransform gpsTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    //GPS not updating underwater
    Liquid* liq = SimulationApp::getApp()->getSimulationManager()->getLiquid();
    if(liq != NULL && liq->IsInsideFluid(gpsTrans.getOrigin()))
        return; 
    
    btScalar x = gpsTrans.getOrigin().getX();
    btScalar y = gpsTrans.getOrigin().getY();
    btScalar d = btSqrt(x*x + y*y);
    btScalar R = 6378.1e3;
    btScalar latitude = btAsin( btSin(homeLatitude) * btCos(d/R) + btCos(homeLatitude) * btSin(d/R) * x/y); //x/y = cos(theta)
    btScalar longitude = homeLongitude + btAtan2( y/x * btSin(d/R) * btCos(homeLatitude), btCos(d/R) - btSin(homeLatitude) * btSin(latitude) ); //y/x = sin(theta)
        
    //Record sample
    btScalar data[2] = {latitude, longitude};
    Sample s(2, data);
    AddSampleToHistory(s);
}

void GPS::SetNoise(btScalar latDev, btScalar longDev)
{
    channels[0].setStdDev(latDev);
    channels[1].setStdDev(longDev);
}