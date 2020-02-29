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
//  Comm.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 25/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "comms/Comm.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

Comm::Comm(std::string uniqueName, uint64_t deviceId, Scalar frequency)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    id = deviceId;
    cId = 0;
    freq = frequency == Scalar(0) ? Scalar(1) : frequency;
    eleapsedTime = Scalar(0);
    renderable = false;
    newDataAvailable = false;
    updateMutex = SDL_CreateMutex();
}

Comm::~Comm()
{
    if(SimulationApp::getApp() != NULL)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
    SDL_DestroyMutex(updateMutex);
}

Transform Comm::getDeviceFrame()
{
    if(attach != NULL)
        return attach->getOTransform() * o2c;
    else
        return o2c;
}

std::string Comm::getName()
{
    return name;
}

uint64_t Comm::getDeviceId()
{
    return id;
}

uint64_t Comm::getConnectedId()
{
    return cId;
}

void Comm::MarkDataOld()
{
    newDataAvailable = false;
}

void Comm::setUpdateFrequency(Scalar f)
{
    freq = f == Scalar(0) ? Scalar(1) : f;
}

bool Comm::isNewDataAvailable()
{
    return newDataAvailable;
}

void Comm::setRenderable(bool render)
{
    renderable = render;
}

bool Comm::isRenderable()
{
    return renderable;
}

void Comm::Connect(uint64_t deviceId)
{
    cId = deviceId;
}

void Comm::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        o2c = origin;
        attach = solid;
    }
}

void Comm::Update(Scalar dt)
{
    SDL_LockMutex(updateMutex);
    
    if(freq <= Scalar(0.)) // Every simulation tick
    {
        InternalUpdate(dt);
        newDataAvailable = true;
    }
    else //Fixed rate
    {
        eleapsedTime += dt;
        Scalar invFreq = Scalar(1.)/freq;
        
        if(eleapsedTime >= invFreq)
        {
            InternalUpdate(invFreq);
            eleapsedTime -= invFreq;
            newDataAvailable = true;
        }
    }
    
    SDL_UnlockMutex(updateMutex);
}

std::vector<Renderable> Comm::Render()
{
    std::vector<Renderable> items(0);
    return items;
}
    
}
