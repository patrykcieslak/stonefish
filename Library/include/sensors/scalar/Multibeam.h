//
//  Multibeam.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Multibeam__
#define __Stonefish_Multibeam__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //!
    class Multibeam : public LinkSensor
    {
    public:
        Multibeam(std::string uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        void InternalUpdate(Scalar dt);
        void SetRange(Scalar distanceMin, Scalar distanceMax);
        void SetNoise(Scalar distanceStdDev);
        std::vector<Renderable> Render();
        
    private:
        Scalar angRange;
        unsigned int angSteps;
        std::vector<Scalar> angles;
        std::vector<Scalar> distances;
    };
}

#endif
