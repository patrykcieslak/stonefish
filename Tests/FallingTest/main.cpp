//
//  main.cpp
//  FallingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "FallingTestApp.h"
#include "FallingTestManager.h"

int main(int argc, const char * argv[])
{
    FallingTestManager* simulationManager = new FallingTestManager(200.0);
    FallingTestApp app("/home/pcieslak/Documents/stonefish/Library/data", 1200, 900, simulationManager);
    app.Run();
	
    return 0;
}

