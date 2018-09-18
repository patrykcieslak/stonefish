//
//  main.cpp
//  JointsTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"
#include "JointsTestManager.h"

int main(int argc, const char * argv[])
{
    JointsTestManager* simulationManager = new JointsTestManager(500.0);
    JointsTestApp app("../../../../Library/data", 1600, 1200, simulationManager);
    app.Run();
    
    return 0;
}

