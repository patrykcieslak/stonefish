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
    sf::RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = sf::RenderQuality::QUALITY_MEDIUM;
    s.ao = sf::RenderQuality::QUALITY_DISABLED;
    s.atmosphere = sf::RenderQuality::QUALITY_MEDIUM;
    s.ocean = sf::RenderQuality::QUALITY_DISABLED;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    //h.showSensors = true;
    h.showActuators = false;
    h.showForces = true;
    
    AcrobotTestManager* simulationManager = new AcrobotTestManager(2000.0);
    AcrobotTestApp app("/home/pcieslak/Documents/stonefish/Tests/Data/", s, h, simulationManager);
    app.Run();
    
    return 0;
}

