//
//  main.cpp
//  FallingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "FallingTestApp.h"
#include "FallingTestManager.h"

int main(int argc, const char * argv[])
{
    RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = RenderQuality::QUALITY_MEDIUM;
    s.ao = RenderQuality::QUALITY_DISABLED;
    s.atmosphere = RenderQuality::QUALITY_MEDIUM;
    s.ocean = RenderQuality::QUALITY_DISABLED;
    
    FallingTestManager* simulationManager = new FallingTestManager(60.0);
    FallingTestApp app("/home/pcieslak/Documents/stonefish/Library/data", s, simulationManager);
    app.Run(false);
	
    return 0;
}

