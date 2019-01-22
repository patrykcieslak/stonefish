//
//  main.cpp
//  JointsTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"
#include "JointsTestManager.h"

int main(int argc, const char * argv[])
{
    sf::RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = sf::RenderQuality::QUALITY_HIGH;
    s.ao = sf::RenderQuality::QUALITY_DISABLED;
    s.atmosphere = sf::RenderQuality::QUALITY_MEDIUM;
    s.ocean = sf::RenderQuality::QUALITY_DISABLED;
    s.msaa = false;
    
    sf::HelperSettings h;
    h.showJoints = true;
    
    JointsTestManager* simulationManager = new JointsTestManager(200.0);
    JointsTestApp app("/Users/zbuffer/Documents/Projects/Stonefish/stonefish/Library/data/",
                      s, h, simulationManager);
    app.Run(false);
    
    return 0;
}

