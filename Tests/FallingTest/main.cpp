//
//  main.cpp
//  FallingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "SimulationApp.h"
#include "SimulationManager.h"

int main(int argc, const char * argv[])
{
    SimulationManager* simulationManager = new SimulationManager(60.0);
    SimulationApp app("Falling objects test", 800, 600, simulationManager);
    app.Init("Data", "Shaders");
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

