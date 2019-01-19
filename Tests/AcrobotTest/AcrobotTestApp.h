//
//  AcrobotTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcrobotTestApp__
#define __Stonefish__AcrobotTestApp__

#include <core/GraphicalSimulationApp.h>
#include "AcrobotTestManager.h"

class AcrobotTestApp : public sf::GraphicalSimulationApp
{
public:
    AcrobotTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, AcrobotTestManager* sim);

    void DoHUD();
};

#endif
