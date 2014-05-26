//
//  Trajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trajectory__
#define __Stonefish_Trajectory__

#include "Sensor.h"
#include "SolidEntity.h"

class Trajectory : public Sensor
{
public:
    Trajectory(std::string uniqueName, SolidEntity* attachment, btVector3 offset = btVector3(0,0,0), unsigned int historyLength = 1);
    
    void Reset();
    void Update(btScalar dt);
    unsigned short getNumOfDimensions();
    void Render();
    
private:
    //parameters
    SolidEntity* solid;
    btVector3 relToCOG;
};


#endif
