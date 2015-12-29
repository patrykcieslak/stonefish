//
//  AcroverSimApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcroverSimApp__
#define __Stonefish__AcroverSimApp__

#include "SimulationApp.h"
#include "AcroverSimManager.h"

class AcroverSimApp : public SimulationApp
{
public:
    AcroverSimApp(int width, int height, AcroverSimManager* sim);

    void DoHUD();
    
    void ProcessInputs();
    void MouseDown(SDL_Event* event);
    void MouseUp(SDL_Event* event);
    void MouseMove(SDL_Event* event);
    void MouseScroll(SDL_Event* event);
    
private:
    btScalar turning;
    btScalar speed;
};

#endif
