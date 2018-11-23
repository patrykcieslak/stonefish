//
//  FallingTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__FallingTestApp__
#define __Stonefish__FallingTestApp__

#include <core/GraphicalSimulationApp.h>
#include "FallingTestManager.h"

class FallingTestApp : public sf::GraphicalSimulationApp
{
public:
    FallingTestApp(std::string dataDirPath, sf::RenderSettings s, FallingTestManager* sim);
    void DoHUD();
    
private:
    bool checked;
};

#endif
