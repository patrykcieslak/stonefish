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
#include "sensors/scalar/JointSensor.h"

namespace sf
{

class Torque : public JointSensor
{
public:
    Torque(std::string uniqueName, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(btScalar max);
    void SetNoise(btScalar stdDev);
};

}

#endif
