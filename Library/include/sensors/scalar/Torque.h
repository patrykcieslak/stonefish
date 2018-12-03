//
//  Torque.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/03/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Torque__
#define __Stonefish_Torque__

#include "sensors/scalar/JointSensor.h"

namespace sf
{
    //!
    class Torque : public JointSensor
    {
    public:
        Torque(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        void InternalUpdate(Scalar dt);
        void SetRange(Scalar max);
        void SetNoise(Scalar stdDev);
    };
}

#endif
