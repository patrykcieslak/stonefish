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
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
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

Sensor::Sensor(std::string uniqueName, Scalar frequency)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    setUpdateFrequency(frequency);
    eleapsedTime = Scalar(0);
    enabled = true;
    renderable = true;
    newDataAvailable = false;
    updateMutex = SDL_CreateMutex();
    lookId = -1;
    graObjectId = -1;
}

Sensor::~Sensor()
{
    if(SimulationApp::getApp() != NULL)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
    SDL_DestroyMutex(updateMutex);
}

std::string Sensor::getName() const
{
    return name;
}

Scalar Sensor::getUpdateFrequency() const
{
    return freq;
}

bool Sensor::isNewDataAvailable() const
{
    return newDataAvailable;
}

bool Sensor::isRenderable() const
{
    return renderable;
}

bool Sensor::isEnabled() const
{
    return enabled;
}

void Sensor::MarkDataOld()
{
    newDataAvailable = false;
}

void Sensor::setUpdateFrequency(Scalar f)
{
    freq = f;
}

void Sensor::setEnabled(bool en)
{
    enabled = en;
}

void Sensor::setRenderable(bool render)
{
    renderable = render;
}

void Sensor::setVisual(const std::string& meshFilename, Scalar scale, const std::string& look)
{
    if(!SimulationApp::getApp()->hasGraphics())
        return;

    Mesh* mesh = OpenGLContent::LoadMesh(meshFilename, scale, false);
    if(mesh == nullptr)
        return;

    graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(mesh);
    lookId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    delete mesh;
}

void Sensor::Reset()
{
    eleapsedTime = Scalar(0.);
    InternalUpdate(1.); //time delta should not affect initial measurement!!!
}

void Sensor::Update(Scalar dt)
{
    if(!enabled)
        return;
        
    SDL_LockMutex(updateMutex);
    
    if(freq <= Scalar(0)) // Every simulation tick
    {
        InternalUpdate(dt);
        newDataAvailable = true;
    }
    else //Fixed rate
    {
        eleapsedTime += dt;
        Scalar invFreq = Scalar(1)/freq;
        
        if(eleapsedTime >= invFreq)
        {
            InternalUpdate(invFreq);
            eleapsedTime -= invFreq;
            newDataAvailable = true;
        }
    }
    
    SDL_UnlockMutex(updateMutex);
}

std::vector<Renderable> Sensor::Render()
{
    std::vector<Renderable> items(0);
    if(renderable && graObjectId > 0)
    {
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = "";
        item.objectId = graObjectId;
        item.lookId = lookId;
        item.model = glMatrixFromTransform(getSensorFrame());
        items.push_back(item);
    }
    return items;
}
    
}
