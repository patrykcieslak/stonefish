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

MovingEntity::MovingEntity(std::string_view uniqueName, std::string_view material, std::string_view look) : Entity(uniqueName)
{
    mat_ = SimulationApp::GetApp()->GetSimulationManager()->GetMaterialManager()->GetMaterial(material);
    if(SimulationApp::GetApp()->HasGraphics())
        lookId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::GetApp())->GetGlPipeline()->GetContent()->GetLookId(look);
    else
        lookId_ = -1;
    graObjectId_ = -1;
    dm_ = DisplayMode::GRAPHICAL;
    particles_.reset();
}

Material MovingEntity::GetMaterial() const
{
    return mat_;
}

void MovingEntity::SetLinearAcceleration(Vector3 a)
{
    linearAcc_ = a;
}
        
void MovingEntity::SetAngularAcceleration(Vector3 epsilon)
{
    angularAcc_ = epsilon;
}

void MovingEntity::SetDisplayMode(DisplayMode m)
{
    dm_ = m;
}

void MovingEntity::SetLook(int newLookId)
{
    lookId_ = newLookId;
}

int MovingEntity::GetLook() const
{
    return lookId_;
}

int MovingEntity::GetGraphicalObject() const
{
    return graObjectId_;
}

const std::shared_ptr<OpenGLOceanParticles>& MovingEntity::GetOceanParticles()
{
    if(particles_ == nullptr 
        && SimulationApp::GetApp()->HasGraphics() 
        && SimulationApp::GetApp()->GetSimulationManager()->IsOceanEnabled())
    {
        particles_ = std::make_shared<OpenGLOceanParticles>(STD_OCEAN_PARTICLES_COUNT, STD_OCEAN_PARTICLES_RADIUS);
    }

    return particles_;
}

btRigidBody* MovingEntity::GetRigidBody()
{
    return rigidBody_.get();
}

}
