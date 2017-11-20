//
//  main.cpp
//  UnderwaterTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestApp.h"
#include "UnderwaterTestManager.h"
#include <fenv.h>

int main(int argc, const char * argv[])
{
    //feenableexcept(FE_ALL_EXCEPT & ~FE_INEXACT);
    feenableexcept(FE_INVALID | FE_OVERFLOW);
    
    UnderwaterTestManager* simulationManager = new UnderwaterTestManager(500.0);
    UnderwaterTestApp app("/home/zbuffer/Documents/stonefish/Library/data/", 1500, 1000, simulationManager);
    app.Init();
    app.EventLoop();
    app.CleanUp();
    
    return 0;
}

