//
//  UnderwaterTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestApp__
#define __Stonefish__UnderwaterTestApp__

#include "SimulationApp.h"
#include "UnderwaterTestManager.h"

class UnderwaterTestApp : public SimulationApp
{
public:
    UnderwaterTestApp(int width, int height, UnderwaterTestManager* sim);
    
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
    
    void DoHUD();
};

#endif
