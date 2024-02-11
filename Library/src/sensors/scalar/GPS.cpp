/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  GPS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/GPS.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/NED.h"
#include "entities/forcefields/Ocean.h"
#include "sensors/Sample.h"

namespace sf
{

GPS::GPS(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Latitude", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Longitude", QuantityType::ANGLE));
    channels.push_back(SensorChannel("North", QuantityType::LENGTH));
    channels.push_back(SensorChannel("East", QuantityType::LENGTH));
    setNoise(Scalar(0));
}

void GPS::InternalUpdate(Scalar dt)
{
    //get sensor frame in world
    Transform gpsTrans = getSensorFrame();
    
    //GPS not updating underwater
    Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(liq != nullptr && liq->IsInsideFluid(gpsTrans.getOrigin()))
    {
        Scalar data[4] = {BT_LARGE_FLOAT, 
                          BT_LARGE_FLOAT, 
                          Scalar(0), Scalar(0)};
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
        SimulationApp::getApp()->getSimulationManager()->getNED()->Ned2Geodetic(gpsPos.x(), gpsPos.y(), 0.0, latitude, longitude, height);
        
        //record sample
        Scalar data[4] = {latitude, longitude, gpsPos.x(), gpsPos.y()};
        Sample s(4, data);
        AddSampleToHistory(s);
    }
}

void GPS::setNoise(Scalar nedDev)
{
    nedStdDev = btClamped(nedDev, Scalar(0), Scalar(BT_LARGE_FLOAT));
    noise = std::normal_distribution<Scalar>(Scalar(0), nedStdDev);
}

Scalar GPS::getNoise()
{
    return nedStdDev;
}

ScalarSensorType GPS::getScalarSensorType()
{
    return ScalarSensorType::GPS;
}

}
