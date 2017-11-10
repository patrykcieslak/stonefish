//
//  GPS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GPS__
#define __Stonefish_GPS__

#include "SimpleSensor.h"
#include "SolidEntity.h"

class GPS : public SimpleSensor
{
public:
    GPS(std::string uniqueName, btScalar latitude, btScalar longitude, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void SetNoise(btScalar latDev, btScalar longDev);
    btTransform getSensorFrame();
    
private:
    SolidEntity* attach;
    btScalar homeLatitude;
    btScalar homeLongitude;
};

#endif