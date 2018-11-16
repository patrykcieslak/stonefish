//
//  main.cpp
//  JointsTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"
#include "JointsTestManager.h"

int main(int argc, const char * argv[])
{
    RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = RenderQuality::QUALITY_MEDIUM;
    s.ao = RenderQuality::QUALITY_DISABLED;
    s.atmosphere = RenderQuality::QUALITY_MEDIUM;
    s.ocean = RenderQuality::QUALITY_DISABLED;
    
    JointsTestManager* simulationManager = new JointsTestManager(500.0);
    JointsTestApp app("../../../../Library/data", s, simulationManager);
    app.Run();
    
    return 0;
}

