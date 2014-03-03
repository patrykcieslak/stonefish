//
//  Entity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "Entity.h"


Entity::Entity(std::string uniqueName)
{
    renderable = true;
    name = uniqueName;
}

Entity::~Entity(void)
{
}

void Entity::setRenderable(bool render)
{
    renderable = render;
}

bool Entity::isRenderable()
{
    return renderable;
}

std::string Entity::getName()
{
    return name;
}