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
    FallingTestApp app(1200, 900, simulationManager);
#ifdef __linux__
    app.Init("../../../../Library/data", "../../../../Library/shaders");
#else
    app.Init("Data", "Shaders");
#endif
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

