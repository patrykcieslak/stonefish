//
//  main.cpp
//  Acrover
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcroverSimApp.h"
#include "AcroverSimManager.h"

int main(int argc, const char * argv[])
{
    AcroverSimManager* simulationManager = new AcroverSimManager(5000.0);
    AcroverSimApp app(1600, 1000, simulationManager);
    app.Init("Data", "Shaders");
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

