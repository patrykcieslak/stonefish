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
//  Visual.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/07/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "visuals/Visual.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/NameManager.h"
#include "entities/SolidEntity.h"
#include "entities/StaticEntity.h"
#include "entities/AnimatedEntity.h"

namespace sf
{

Visual::Visual(std::string uniqueName) : attach(nullptr), attach2(nullptr), attach3(nullptr)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use visuals in console simulation! Use graphical simulation if possible.");

    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
}

Visual::~Visual()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

std::string Visual::getName() const
{
    return name;
}
 
Transform Visual::getVisualFrame() const
{
	if(attach != nullptr)
        return attach->getOTransform() * o2a; //Solid
    else if(attach2 != nullptr)
		return attach2->getTransform() * o2a; //Static
	else if(attach3 != nullptr)
        return attach3->getOTransform() * o2a; //Animated
    else
        return o2a;
}

void Visual::AttachToWorld(const Transform& origin)
{
	o2a = origin;
    attach = nullptr;
    attach2 = nullptr;
    attach3 = nullptr;
}
        
void Visual::AttachToStatic(StaticEntity* body, const Transform& origin)
{
	if(body != nullptr)
	{
		o2a = origin;
		attach = nullptr;
        attach2 = body;
        attach3 = nullptr;
	}
}

void Visual::AttachToAnimated(AnimatedEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2a = origin;
        attach = nullptr;
        attach2 = nullptr;
        attach3 = body;
    }
}

void Visual::AttachToSolid(SolidEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2a = origin;
        attach = body;
        attach2 = nullptr;
        attach3 = nullptr;
    }
}
    
}
