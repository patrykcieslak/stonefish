//
//  JointsTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__JointsTestApp__
#define __Stonefish__JointsTestApp__

#include "SimulationApp.h"
#include "JointsTestManager.h"

class JointsTestApp : public SimulationApp
{
public:
    JointsTestApp(int width, int height, JointsTestManager* sim);
    
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
};

#endif
