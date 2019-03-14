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
    
    sf::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.msaa = false;
    s.shadows = sf::RenderQuality::QUALITY_HIGH;
    s.ao = sf::RenderQuality::QUALITY_LOW;
    s.atmosphere = sf::RenderQuality::QUALITY_MEDIUM;
    s.ocean = sf::RenderQuality::QUALITY_HIGH;
    
    sf::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = true;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = true;
    h.showForces = false;
    
    UnderwaterTestManager* simulationManager = new UnderwaterTestManager(200.0);
    UnderwaterTestApp app("/home/pcieslak/Documents/stonefish/Tests/Data/", s, h, simulationManager);
    //UnderwaterTestApp app("/Users/zbuffer/Documents/Projects/Stonefish/stonefish/Tests/Data/", s, h, simulationManager);
    app.Run(false);
    
    return 0;
}

