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
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/MovingEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLOceanParticles.h"

namespace sf
{

MovingEntity::MovingEntity(const std::string& uniqueName, const std::string& material, const std::string& look) : Entity(uniqueName)
{
    rigidBody_ = nullptr;
    mat_ = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(material);
    if(SimulationApp::getApp()->hasGraphics())
        lookId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    else
        lookId_ = -1;
    graObjectId_ = -1;
    dm_ = DisplayMode::GRAPHICAL;
    particles_.reset();
}

Material MovingEntity::getMaterial() const
{
    return mat_;
}

void MovingEntity::setLinearAcceleration(Vector3 a)
{
    linearAcc_ = a;
}
        
void MovingEntity::setAngularAcceleration(Vector3 epsilon)
{
    angularAcc_ = epsilon;
}

void MovingEntity::setDisplayMode(DisplayMode m)
{
    dm_ = m;
}

void MovingEntity::setLook(int newLookId)
{
    lookId_ = newLookId;
}

int MovingEntity::getLook() const
{
    return lookId_;
}

int MovingEntity::getGraphicalObject() const
{
    return graObjectId_;
}

const std::shared_ptr<OpenGLOceanParticles>& MovingEntity::getOceanParticles()
{
    if(particles_ == nullptr 
        && SimulationApp::getApp()->hasGraphics() 
        && SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
    {
        particles_ = std::make_shared<OpenGLOceanParticles>(STD_OCEAN_PARTICLES_COUNT, STD_OCEAN_PARTICLES_RADIUS);
    }

    return particles_;
}

btRigidBody* MovingEntity::getRigidBody()
{
    return rigidBody_;
}

}
