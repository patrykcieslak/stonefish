//
//  main.cpp
//  SlidingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestApp.h"
#include "SlidingTestManager.h"

int main(int argc, const char * argv[])
{
    SlidingTestManager* simulationManager = new SlidingTestManager(500.0);
    SlidingTestApp app(1000, 700, simulationManager);
    app.Init("Data", "Shaders");
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

