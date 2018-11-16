//
//  SlidingTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__SlidingTestApp__
#define __Stonefish__SlidingTestApp__

#include <core/GraphicalSimulationApp.h>
#include "SlidingTestManager.h"

class SlidingTestApp : public GraphicalSimulationApp
{
public:
    SlidingTestApp(std::string dataDirPath, RenderSettings s, SlidingTestManager* sim);
    
    void DoHUD();
};

#endif
