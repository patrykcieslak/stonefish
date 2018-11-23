//
//  Current.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Current__
#define __Stonefish_Current__

#include "sensors/ScalarSensor.h"
#include "actuators/DCMotor.h"

namespace sf
{

class Current : public ScalarSensor
{
public:
    Current(std::string uniqueName, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void AttachToMotor(DCMotor* m);
    SensorType getType();
   
private:
    DCMotor* motor;
};

}

#endif
