//
//  UnderwaterTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestApp__
#define __Stonefish__UnderwaterTestApp__

#include "SimulationApp.h"
#include "UnderwaterTestManager.h"

class UnderwaterTestApp : public SimulationApp
{
public:
    UnderwaterTestApp(std::string dataDirPath, std::string shaderDirPath, int width, int height, UnderwaterTestManager* sim);
    void DoHUD();
};

#endif
