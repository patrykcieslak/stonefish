//
//  FallingTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__FallingTestApp__
#define __Stonefish__FallingTestApp__

#include "SimulationApp.h"
#include "FallingTestManager.h"

class FallingTestApp : public SimulationApp
{
public:
    FallingTestApp(std::string dataDirPath, std::string shaderDirPath, int width, int height, FallingTestManager* sim);
    void DoHUD();
    
private:
    bool checked;
};

#endif
