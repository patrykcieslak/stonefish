//
//  Trajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trajectory__
#define __Stonefish_Trajectory__

#include "sensors/SimpleSensor.h"

class Trajectory : public SimpleSensor
{
public:
    Trajectory(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    virtual std::vector<Renderable> Render();
    
private:
    SolidEntity* solid;
};


#endif
