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
    sf::RenderSettings s;
    s.windowW = 800;
    s.windowH = 600;
    s.shadows = sf::RenderQuality::QUALITY_MEDIUM;
    s.ao = sf::RenderQuality::QUALITY_DISABLED;
    s.atmosphere = sf::RenderQuality::QUALITY_LOW;
    s.ocean = sf::RenderQuality::QUALITY_DISABLED;
    
    FallingTestManager* simulationManager = new FallingTestManager(60.0);
    FallingTestApp app(DATA_DIR_PATH, s, simulationManager);
    app.Run(false);
	
    return 0;
}

