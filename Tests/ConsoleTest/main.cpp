//
//  main.cpp
//  ConsoleTest
//
//  Created by Patryk Cieslak on 12/09/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ConsoleSimulationApp.h"
#include "ConsoleTestManager.h"

int main(int argc, const char * argv[])
{
    ConsoleTestManager* simulationManager = new ConsoleTestManager(500.0);
    ConsoleSimulationApp app("Console Test", "/home/pcieslak/Documents/stonefish/Library/data/", simulationManager);
    app.Run();
    
    return 0;
}