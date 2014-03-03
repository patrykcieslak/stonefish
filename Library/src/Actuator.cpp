//
//  Actuator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Actuator.h"

Actuator::Actuator()
{
    renderable = false;
}

Actuator::~Actuator()
{
}

void Actuator::setRenderable(bool render)
{
    renderable = render;
}

bool Actuator::isRenderable()
{
    return renderable;
}