//
//  FakeIMU.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FakeIMU__
#define __Stonefish_FakeIMU__

#include "IMU.h"

class FakeIMU : public IMU
{
public:
    FakeIMU(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    
private:
    btVector3 lastV;
};

#endif
