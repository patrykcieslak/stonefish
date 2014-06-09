//
//  FallingTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__FallingTestApp__
#define __Stonefish__FallingTestApp__

#include "SimulationApp.h"
#include "FallingTestManager.h"

class FallingTestApp : public SimulationApp
{
public:
    FallingTestApp(int width, int height, FallingTestManager* sim);
    
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
    
    void DoHUD();
    
private:
    bool checked;
    unsigned short radioOption;
};

#endif
