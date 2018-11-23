//
//  GPS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/GPS.h"

#include "core/SimulationApp.h"

using namespace sf;

GPS::GPS(std::string uniqueName, btScalar latitudeDeg, btScalar longitudeDeg, btScalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
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
    //get sensor frame in world
    btTransform gpsTrans = getSensorFrame();
    
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
		
        //add noise
        if(!btFuzzyZero(nedStdDev))
        {
            gpsPos.setX(gpsPos.x() + noise(randomGenerator));
            gpsPos.setY(gpsPos.y() + noise(randomGenerator));
        }
        
        //convert NED to geodetic coordinates
        double latitude;
        double longitude;
        double height;
        ned->ned2Geodetic(gpsPos.x(), gpsPos.y(), 0.0, latitude, longitude, height);
        
        //record sample
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
