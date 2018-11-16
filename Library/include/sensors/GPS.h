//
//  GPS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GPS__
#define __Stonefish_GPS__

#include "sensors/SimpleSensor.h"
#include "sensors/NED.h"

class GPS : public SimpleSensor
{
public:
    GPS(std::string uniqueName, btScalar latitudeDeg, btScalar longitudeDeg, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    ~GPS();
    
    void InternalUpdate(btScalar dt);
    void SetNoise(btScalar nedDev);
    btTransform getSensorFrame();
    
private:
    SolidEntity* attach;
    NED* ned;
    
    //Custom noise generation specific to GPS 
    btScalar nedStdDev;
    std::normal_distribution<btScalar> noise;
};

#endif
