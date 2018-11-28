//
//  Actuator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/Actuator.h"

#include "core/SimulationApp.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Actuator::Actuator(std::string uniqueName)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
}

Actuator::~Actuator()
{
    SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

std::string Actuator::getName()
{
    return name;
}

std::vector<Renderable> Actuator::Render()
{
    std::vector<Renderable> items(0);
    return items;
}

}
