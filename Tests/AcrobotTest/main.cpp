//
//  main.cpp
//  AcrobotTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "AcrobotTestApp.h"
#include "AcrobotTestManager.h"

int main(int argc, const char * argv[])
{
    AcrobotTestManager* simulationManager = new AcrobotTestManager(2000.0);
    AcrobotTestApp app("/home/pcieslak/Documents/stonefish/Library/data/", 1100, 700, simulationManager);
    app.Run();
    
    return 0;
}

