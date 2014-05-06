//
//  main.cpp
//  RollingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RollingTestApp.h"
#include "RollingTestManager.h"

int main(int argc, const char * argv[])
{
    RollingTestManager* simulationManager = new RollingTestManager(100.0);
    RollingTestApp app(1100, 700, simulationManager);
    app.Init("Data", "Shaders");
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

