//
//  main.cpp
//  SlidingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestApp.h"
#include "SlidingTestManager.h"

int main(int argc, const char * argv[])
{
    RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = RenderQuality::QUALITY_MEDIUM;
    s.ao = RenderQuality::QUALITY_DISABLED;
    s.atmosphere = RenderQuality::QUALITY_MEDIUM;
    s.ocean = RenderQuality::QUALITY_DISABLED;
    
    SlidingTestManager* simulationManager = new SlidingTestManager(500.0);
    SlidingTestApp app("/home/pcieslak/Documents/stonefish/Library/data", s, simulationManager);
    app.Run(false);
    
    return 0;
}

