//
//  GPS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "sensors/GPS.h"

#include "core/SimulationApp.h"

GPS::GPS(std::string uniqueName, btScalar latitudeDeg, btScalar longitudeDeg, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    ned = new NED(latitudeDeg, longitudeDeg, btScalar(0.0));
    nedStdDev = btScalar(0);
    
    channels.push_back(SensorChannel("Latitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Longitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("North", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("East", QUANTITY_LENGTH));
}

GPS::~GPS()
{
    delete ned;
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
        btVector3 gpsPos = gpsTrans.getOrigin();
		
        //Add noise
        if(!btFuzzyZero(nedStdDev))
        {
            gpsPos.setX(gpsPos.x() + noise(randomGenerator));
            gpsPos.setY(gpsPos.y() + noise(randomGenerator));
        }
        
        //Convert NED to geodetic coordinates
        double latitude;
        double longitude;
        double height;
        ned->ned2Geodetic(gpsPos.x(), gpsPos.y(), 0.0, latitude, longitude, height);
        
        //Record sample
        btScalar data[4] = {latitude, longitude, gpsPos.x(), gpsPos.y()};
        Sample s(4, data);
        
        
        AddSampleToHistory(s);
    }
}

void GPS::SetNoise(btScalar nedDev)
{
    nedStdDev = nedDev > btScalar(0) ? nedDev : btScalar(0);
    noise = std::normal_distribution<btScalar>(btScalar(0), nedStdDev);
}

btTransform GPS::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}
