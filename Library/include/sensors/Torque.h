//
//  Torque.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Torque__
#define __Stonefish_Torque__

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "sensors/SimpleSensor.h"

class Torque : public SimpleSensor
{
public:
    //Torque(std::string uniqueName, Joint* j, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency = btScalar(-1.), int historyLength = -1);
    Torque(std::string uniqueName, FeatherstoneEntity* f, unsigned int jointId, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    std::vector<Renderable> Render();
    
    void Reset();
    void SetRange(btScalar max);
    void SetNoise(btScalar stdDev);
    
private:
    //Joint* joint;
    //SolidEntity* attach;
    FeatherstoneEntity* fe;
    unsigned int jId;
};

#endif
