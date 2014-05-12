//
//  AcrobotTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestApp__
#define __Stonefish__AcrobotTestApp__

#include "SimulationApp.h"
#include "AcrobotTestManager.h"

class AcrobotTestApp : public SimulationApp
{
public:
    AcrobotTestApp(int width, int height, AcrobotTestManager* sim);

    void DoHUD();
    
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
};

#endif
