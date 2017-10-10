//
//  Actuator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Actuator.h"

NameManager Actuator::nameManager;

Actuator::Actuator(std::string uniqueName)
{
    name = nameManager.AddName(uniqueName);
}

Actuator::~Actuator()
{
    nameManager.RemoveName(name);
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