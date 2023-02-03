/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Entity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2023 Patryk Cieslak. All rights reserved.
//

#include "entities/Entity.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Entity::Entity(std::string uniqueName)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    renderable = true;
}

Entity::~Entity(void)
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

void Entity::setRenderable(bool render)
{
    renderable = render;
}

bool Entity::isRenderable() const
{
    return renderable;
}

std::string Entity::getName() const
{
    return name;
}
        
}
