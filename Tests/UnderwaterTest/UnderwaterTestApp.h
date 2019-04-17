//
//  UnderwaterTestApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__UnderwaterTestApp__
#define __Stonefish__UnderwaterTestApp__

#include <core/GraphicalSimulationApp.h>
#include <graphics/OpenGLPrinter.h>
#include "UnderwaterTestManager.h"

#define USE_IAUV_CLASSES

class UnderwaterTestApp : public sf::GraphicalSimulationApp
{
public:
    UnderwaterTestApp(std::string dataDirPath, sf::RenderSettings s, sf::HelperSettings h, UnderwaterTestManager* sim);
    
    void DoHUD();
    void InitializeGUI();
    
private:
    sf::OpenGLPrinter* largePrint;
};

#endif
