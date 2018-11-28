//
//  Trajectory.h
//  Stonefish
//
//  Created by Patryk Cieslak on 25/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trajectory__
#define __Stonefish_Trajectory__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{

class Trajectory : public LinkSensor
{
public:
    Trajectory(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void InternalUpdate(Scalar dt);
    std::vector<Renderable> Render();
};

}

#endif
