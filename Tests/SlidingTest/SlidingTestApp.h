//
//  SlidingTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__SlidingTestApp__
#define __Stonefish__SlidingTestApp__

#include "SimulationApp.h"
#include "SlidingTestManager.h"

class SlidingTestApp : public SimulationApp
{
public:
    SlidingTestApp(int width, int height, SlidingTestManager* sim);
    
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
    
    void DoHUD();
};

#endif
