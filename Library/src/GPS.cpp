//
//  GPS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "GPS.h"
#include "SimulationApp.h"

GPS::GPS(std::string uniqueName, btScalar latitude, btScalar longitude, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    homeLatitude = UnitSystem::SetAngle(latitude);
    homeLongitude = UnitSystem::SetAngle(longitude);
    channels.push_back(SensorChannel("Latitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Longitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("North", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("East", QUANTITY_LENGTH));
}

void GPS::InternalUpdate(btScalar dt)
{
    btTransform gpsTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    //GPS not updating underwater
    Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(liq != NULL && liq->IsInsideFluid(gpsTrans.getOrigin()))
    {
        btScalar data[4] = {btScalar(0), btScalar(-1), btScalar(0), btScalar(0)};
        Sample s(4, data);
        AddSampleToHistory(s);
    }
    else
    {
        btScalar x = gpsTrans.getOrigin().getX();
        btScalar y = gpsTrans.getOrigin().getY();
        btScalar d = btSqrt(x*x + y*y);
        btScalar R = 6378.1e3;
        btScalar latitude = btAsin( btSin(homeLatitude) * btCos(d/R) + btCos(homeLatitude) * btSin(d/R) * x/y); //x/y = cos(theta)
        btScalar longitude = homeLongitude + btAtan2( y/x * btSin(d/R) * btCos(homeLatitude), btCos(d/R) - btSin(homeLatitude) * btSin(latitude) ); //y/x = sin(theta)
        
        //Record sample
        btScalar data[4] = {latitude, longitude, x, y};
        Sample s(4, data);
        AddSampleToHistory(s);
    }
}

void GPS::SetNoise(btScalar latDev, btScalar longDev)
{
    channels[0].setStdDev(latDev);
    channels[1].setStdDev(longDev);
}

btTransform GPS::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}