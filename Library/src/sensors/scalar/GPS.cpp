//
//  GPS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/GPS.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/NED.h"
#include "entities/forcefields/Ocean.h"
#include "sensors/Sample.h"

namespace sf
{

GPS::GPS(std::string uniqueName, Scalar latitudeDeg, Scalar longitudeDeg, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    ned = new NED(latitudeDeg, longitudeDeg, Scalar(0.0));
    nedStdDev = Scalar(0);
    
    channels.push_back(SensorChannel("Latitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Longitude", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("North", QUANTITY_LENGTH));
    channels.push_back(SensorChannel("East", QUANTITY_LENGTH));
}

GPS::~GPS()
{
    delete ned;
}

void GPS::InternalUpdate(Scalar dt)
{
    //get sensor frame in world
    Transform gpsTrans = getSensorFrame();
    
    //GPS not updating underwater
    Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(liq != NULL && liq->IsInsideFluid(gpsTrans.getOrigin()))
    {
        Scalar data[4] = {Scalar(0), Scalar(-1), Scalar(0), Scalar(0)};
        Sample s(4, data);
        AddSampleToHistory(s);
    }
    else
    {
        Vector3 gpsPos = gpsTrans.getOrigin();
		
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
        Scalar data[4] = {latitude, longitude, gpsPos.x(), gpsPos.y()};
        Sample s(4, data);
        AddSampleToHistory(s);
    }
}

void GPS::setNoise(Scalar nedDev)
{
    nedStdDev = nedDev > Scalar(0) ? nedDev : Scalar(0);
    noise = std::normal_distribution<Scalar>(Scalar(0), nedStdDev);
}

Scalar GPS::getNoise()
{
    return nedStdDev;
}

}
