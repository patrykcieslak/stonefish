//
//  AcrobotTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestApp__
#define __Stonefish__AcrobotTestApp__

#include "GraphicalSimulationApp.h"
#include "AcrobotTestManager.h"

class AcrobotTestApp : public GraphicalSimulationApp
{
public:
    AcrobotTestApp(std::string dataDirPath, int width, int height, AcrobotTestManager* sim);

    void DoHUD();
};

#endif
