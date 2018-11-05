//
//  ConsoleTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__ConsoleTestApp__
#define __Stonefish__ConsoleTestApp__

#include <core/ConsoleSimulationApp.h>
#include "ConsoleTestManager.h"

class ConsoleTestApp : public ConsoleSimulationApp
{
public:
    ConsoleTestApp(std::string dataDirPath, ConsoleTestManager* sim);
	
	void Loop();
};

#endif
