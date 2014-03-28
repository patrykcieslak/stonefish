//
//  main.cpp
//  JointsTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"
#include "JointsTestManager.h"

int main(int argc, const char * argv[])
{
    JointsTestManager* simulationManager = new JointsTestManager(500.0);
    JointsTestApp app(800, 600, simulationManager);
    app.Init("Data", "Shaders");
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

