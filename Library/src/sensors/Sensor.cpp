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
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/6/17.
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/Sensor.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/Console.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

std::random_device Sensor::randomDevice;
std::mt19937 Sensor::randomGenerator(randomDevice());

void Sensor::DefaultDeleter(Sensor* s)
{
    delete s;
}

Sensor::Sensor(const std::string& uniqueName, Scalar frequency)
{
    name_ = SimulationApp::GetApp()->GetSimulationManager()->GetNameManager()->AddName(uniqueName);
    SetUpdateFrequency(frequency);
    eleapsedTime_ = Scalar(0);
    enabled_ = true;
    renderable_ = true;
    newDataAvailable_ = false;
    updateMutex_ = SDL_CreateMutex();
    lookId_ = -1;
    graObjectId_ = -1;
}

Sensor::~Sensor()
{
    if(SimulationApp::GetApp() != NULL)
        SimulationApp::GetApp()->GetSimulationManager()->GetNameManager()->RemoveName(name_);
    SDL_DestroyMutex(updateMutex_);
}

const std::string& Sensor::GetName() const
{
    return name_;
}

Scalar Sensor::GetUpdateFrequency() const
{
    return freq_;
}

bool Sensor::IsNewDataAvailable() const
{
    return newDataAvailable_;
}

bool Sensor::IsRenderable() const
{
    return renderable_;
}

bool Sensor::IsEnabled() const
{
    return enabled_;
}

void Sensor::MarkDataOld()
{
    newDataAvailable_ = false;
}

void Sensor::SetUpdateFrequency(Scalar f)
{
    freq_ = f;
}

void Sensor::SetEnabled(bool en)
{
    enabled_ = en;
}

void Sensor::SetRenderable(bool render)
{
    renderable_ = render;
}

void Sensor::SetVisual(const std::string& meshFilename, Scalar scale, const std::string& look)
{
    if(!SimulationApp::GetApp()->HasGraphics())
        return;

    std::unique_ptr<Mesh> mesh = OpenGLContent::LoadMesh(meshFilename, scale, false);
    if(mesh == nullptr)
        return;

    graObjectId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::GetApp())->GetGlPipeline()->GetContent()->BuildObject(mesh.get());
    lookId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::GetApp())->GetGlPipeline()->GetContent()->GetLookId(look);
}

void Sensor::Reset()
{
    eleapsedTime_ = Scalar(0.);
    InternalUpdate(1.); //time delta should not affect initial measurement!!!
}

void Sensor::Update(Scalar dt)
{
    if(!enabled_)
        return;
        
    SDL_LockMutex(updateMutex_);
    
    if(freq_ <= Scalar(0)) // Every simulation tick
    {
        InternalUpdate(dt);
        newDataAvailable_ = true;
    }
    else //Fixed rate
    {
        eleapsedTime_ += dt;
        Scalar invFreq = Scalar(1)/freq_;
        
        if(eleapsedTime_ >= invFreq)
        {
            InternalUpdate(invFreq);
            eleapsedTime_ -= invFreq;
            newDataAvailable_ = true;
        }
    }
    
    SDL_UnlockMutex(updateMutex_);
}

std::vector<Renderable> Sensor::Render()
{
    std::vector<Renderable> items(0);
    if(renderable_ && graObjectId_ > 0)
    {
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = "";
        item.objectId = graObjectId_;
        item.lookId = lookId_;
        item.model = glMatrixFromTransform(GetSensorFrame());
        items.push_back(item);
    }
    return items;
}
    
}
