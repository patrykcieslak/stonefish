//
//  main.cpp
//  UnderwaterTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"
#include "UnderwaterTestManager.h"
#include <cfenv>

int main(int argc, const char * argv[])
{
    //feenableexcept(FE_ALL_EXCEPT & ~FE_INEXACT);
    //feenableexcept(FE_INVALID | FE_OVERFLOW);
    
    RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = RenderQuality::QUALITY_HIGH;
    s.ao = RenderQuality::QUALITY_HIGH;
    s.atmosphere = RenderQuality::QUALITY_MEDIUM;
    s.ocean = RenderQuality::QUALITY_MEDIUM;
    
    UnderwaterTestManager* simulationManager = new UnderwaterTestManager(500.0);
    UnderwaterTestApp app("/Users/zbuffer/Documents/Projects/Stonefish/stonefish/Library/data/", s, simulationManager);
    //UnderwaterTestApp app("/home/pcieslak/Documents/stonefish/Library/data/", s, simulationManager);
    app.Run(false);
    
    return 0;
}

