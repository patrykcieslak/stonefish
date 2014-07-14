//
//  Current.h
//  Stonefish
//
//  Created by Patryk Cieslak on 09/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Current__
#define __Stonefish_Current__

#include "Sensor.h"
#include "DCMotor.h"

class Current : public Sensor
{
public:
    Current(std::string uniqueName, DCMotor* m, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    
private:
    //params
    DCMotor* motor;
};


#endif
