//
//  RollingTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__RollingTestApp__
#define __Stonefish__RollingTestApp__

#include "SimulationApp.h"
#include "RollingTestManager.h"

class RollingTestApp : public SimulationApp
{
public:
    RollingTestApp(int width, int height, RollingTestManager* sim);
    
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
};

#endif
