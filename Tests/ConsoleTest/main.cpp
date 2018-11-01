//
//  main.cpp
//  ConsoleTest
//
//  Created by Patryk Cieslak on 12/09/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestApp.h"
#include "ConsoleTestManager.h"

int main(int argc, const char * argv[])
{
    ConsoleTestManager* simulationManager = new ConsoleTestManager(500.0);
    ConsoleTestApp app("/home/parallels/Documents/stonefish/Library/data/", simulationManager);
    app.Run(false);
    
    return 0;
}