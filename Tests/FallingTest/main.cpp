//
//  main.cpp
//  FallingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FallingTestApp.h"
#include "FallingTestManager.h"

int main(int argc, const char * argv[])
{
    FallingTestManager* simulationManager = new FallingTestManager(100.0);
    FallingTestApp app(1000, 700, simulationManager);
    app.Init("Data", "Shaders");
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

