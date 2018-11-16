//
//  main.cpp
//  AcrobotTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "AcrobotTestApp.h"
#include "AcrobotTestManager.h"

int main(int argc, const char * argv[])
{
    RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = RenderQuality::QUALITY_MEDIUM;
    s.ao = RenderQuality::QUALITY_DISABLED;
    s.atmosphere = RenderQuality::QUALITY_MEDIUM;
    s.ocean = RenderQuality::QUALITY_DISABLED;
    
    AcrobotTestManager* simulationManager = new AcrobotTestManager(2000.0);
    AcrobotTestApp app("/home/pcieslak/Documents/stonefish/Library/data/", s, simulationManager);
    app.Run();
    
    return 0;
}

