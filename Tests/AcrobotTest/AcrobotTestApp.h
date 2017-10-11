//
//  AcrobotTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestApp__
#define __Stonefish__AcrobotTestApp__

#include "SimulationApp.h"
#include "AcrobotTestManager.h"

class AcrobotTestApp : public SimulationApp
{
public:
    AcrobotTestApp(std::string dataDirPath, std::string shaderDirPath, int width, int height, AcrobotTestManager* sim);

    void DoHUD();
};

#endif
