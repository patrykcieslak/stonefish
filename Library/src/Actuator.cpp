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
    renderable = false;
}

Actuator::~Actuator()
{
    nameManager.RemoveName(name);
}

void Actuator::setRenderable(bool render)
{
    renderable = render;
}

std::string Actuator::getName()
{
    return name;
}

bool Actuator::isRenderable()
{
    return renderable;
}