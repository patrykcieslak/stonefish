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
//  MovingEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/07/2020.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include "entities/MovingEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLOceanParticles.h"
#include <memory>

namespace sf
{

MovingEntity::MovingEntity(std::string uniqueName, std::string material, std::string look) : Entity(uniqueName)
{
    rigidBody = nullptr;
    mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(material);
    if(SimulationApp::getApp()->hasGraphics())
        lookId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    else
        lookId = -1;
    graObjectId = -1;
    dm = DisplayMode::GRAPHICAL;
    particles = nullptr;
}

MovingEntity::~MovingEntity()
{
}

Material MovingEntity::getMaterial() const
{
    return mat;
}

void MovingEntity::setLinearAcceleration(Vector3 a)
{
    linearAcc = a;
}
        
void MovingEntity::setAngularAcceleration(Vector3 epsilon)
{
    angularAcc = epsilon;
}

void MovingEntity::setDisplayMode(DisplayMode m)
{
    dm = m;
}

void MovingEntity::setLook(int newLookId)
{
    lookId = newLookId;
}

int MovingEntity::getLook() const
{
    return lookId;
}

int MovingEntity::getGraphicalObject() const
{
    return graObjectId;
}

std::shared_ptr<OpenGLOceanParticles> MovingEntity::getOceanParticles()
{
    if(particles == nullptr
        && SimulationApp::getApp()->hasGraphics()
        && SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
    {
        particles = std::make_shared<OpenGLOceanParticles>(STD_OCEAN_PARTICLES_COUNT, STD_OCEAN_PARTICLES_RADIUS);
    }

    return particles;
}

btRigidBody* MovingEntity::getRigidBody()
{
    return rigidBody;
}

}
