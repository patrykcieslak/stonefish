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
//  ForcefieldEntity.cpp
//
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2017-2020 Patryk Cieslak. All rights reserved.
//

#include "entities/ForcefieldEntity.h"

#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

ForcefieldEntity::ForcefieldEntity(std::string uniqueName) : Entity(uniqueName)
{
    ghost = new btPairCachingGhostObject();
    ghost->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

ForcefieldEntity::~ForcefieldEntity()
{
}

EntityType ForcefieldEntity::getType() const
{
    return EntityType::FORCEFIELD;
}

btPairCachingGhostObject* ForcefieldEntity::getGhost()
{
    return ghost;
}

void ForcefieldEntity::AddToSimulation(SimulationManager* sm)
{
    sm->getDynamicsWorld()->addCollisionObject(ghost, MASK_GHOST, MASK_DYNAMIC);
}

std::vector<Renderable> ForcefieldEntity::Render()
{
    return std::vector<Renderable>(0);
}

void ForcefieldEntity::getAABB(Vector3& min, Vector3& max)
{
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
}

}
