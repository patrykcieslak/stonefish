//
//  GPS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/11/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GPS__
#define __Stonefish_GPS__

#include "core/NED.h"
#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class GPS : public LinkSensor
{
public:
    GPS(std::string uniqueName, btScalar latitudeDeg, btScalar longitudeDeg, btScalar frequency = btScalar(-1), int historyLength = -1);
    ~GPS();
    
    void InternalUpdate(btScalar dt);
    void SetNoise(btScalar nedDev);
    
private:
    NED* ned;
    
    //Custom noise generation specific to GPS 
    btScalar nedStdDev;
    std::normal_distribution<btScalar> noise;
};
    
}

#endif
