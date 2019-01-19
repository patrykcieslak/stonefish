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
    //!
    class DVL : public LinkSensor
    {
    public:
        DVL(std::string uniqueName, Scalar beamSpreadAngleDeg, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        void InternalUpdate(Scalar dt);
        void SetRange(const Vector3& velocityMax, Scalar altitudeMin, Scalar altitudeMax);
        void SetNoise(Scalar velocityStdDev, Scalar altitudeStdDev);
        std::vector<Renderable> Render();
        
    private:
        Scalar beamAngle;
        Scalar range[4];
    };
}

#endif
