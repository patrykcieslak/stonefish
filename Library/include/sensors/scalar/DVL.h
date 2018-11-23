//
//  DVL.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DVL__
#define __Stonefish_DVL__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class DVL : public LinkSensor
{
public:
    DVL(std::string uniqueName, btScalar beamSpreadAngle, btScalar frequency = btScalar(-1), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void SetRange(const btVector3& velocityMax, btScalar altitudeMin, btScalar altitudeMax);
    void SetNoise(btScalar velocityStdDev, btScalar altitudeStdDev);
    std::vector<Renderable> Render();
    
private:
    btScalar beamAngle;
    btScalar range[4];
};
    
}

#endif
