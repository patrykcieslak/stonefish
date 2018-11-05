//
//  JointsTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__JointsTestApp__
#define __Stonefish__JointsTestApp__

#include <core/GraphicalSimulationApp.h>
#include "JointsTestManager.h"

class JointsTestApp : public GraphicalSimulationApp
{
public:
    JointsTestApp(std::string dataDirPath, int width, int height, JointsTestManager* sim);
};

#endif
