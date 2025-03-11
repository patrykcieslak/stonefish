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
#include "entities/MovingEntity.h"
#include "entities/StaticEntity.h"

namespace sf
{

Comm::Comm(std::string uniqueName, uint64_t deviceId)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    id = deviceId;
    cId = -1;
    renderable = false;
    newDataAvailable = false;
    updateMutex = SDL_CreateMutex();
    attach = nullptr;
    o2c = I4();
    txSeq = 0;
}

Comm::~Comm()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
    SDL_DestroyMutex(updateMutex);
}

Transform Comm::getDeviceFrame()
{
    if(attach != nullptr)
    {
        if(attach->getType() == EntityType::STATIC)
            return ((StaticEntity*)attach)->getTransform() * o2c;
        else if(attach->getType() == EntityType::SOLID || attach->getType() == EntityType::ANIMATED)
            return ((MovingEntity*)attach)->getOTransform() * o2c;
    }
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

void Comm::SendMessage(std::string data)
{
    if(cId > 0)
    {
        CommDataFrame* msg = new CommDataFrame();
        msg->seq = txSeq++;
        msg->source = id;
        msg->destination = cId;
        msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
        msg->data = data;
        txBuffer.push_back(msg);
    }
    else
        return;
}

CommDataFrame* Comm::ReadMessage()
{
    CommDataFrame* msg = nullptr;
    if(rxBuffer.size() > 0)
    {
        msg = rxBuffer[0];
        rxBuffer.pop_front();
    }
    return msg;
}

void Comm::MessageReceived(CommDataFrame* message)
{
    rxBuffer.push_back(message);
}

void Comm::AttachToWorld(const Transform& origin)
{
    o2c = origin;
}

void Comm::AttachToStatic(StaticEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2c = origin;
        attach = body;
    }
}

void Comm::AttachToSolid(MovingEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2c = origin;
        attach = body;
    }
}

void Comm::Update(Scalar dt)
{
    SDL_LockMutex(updateMutex);
    ProcessMessages();
    InternalUpdate(dt);
    SDL_UnlockMutex(updateMutex);
}

std::vector<Renderable> Comm::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.type = RenderableType::SENSOR_CS;
    item.model = glMatrixFromTransform(getDeviceFrame());
    items.push_back(item);
    
    return items;
}
    
}
