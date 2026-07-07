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
    name_ = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    id_ = deviceId;
    cId_ = -1;
    renderable_ = false;
    newDataAvailable_ = false;
    updateMutex_ = SDL_CreateMutex();
    attach_ = nullptr;
    o2c_ = I4();
    txSeq_ = 0;
}

Comm::~Comm()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name_);
    SDL_DestroyMutex(updateMutex_);
}

Transform Comm::getDeviceFrame()
{
    if(attach_ != nullptr)
    {
        if(attach_->getType() == EntityType::STATIC)
            return ((StaticEntity*)attach_)->getTransform() * o2c_;
        else if(attach_->getType() == EntityType::SOLID || attach_->getType() == EntityType::ANIMATED)
            return ((MovingEntity*)attach_)->getOTransform() * o2c_;
    }
    return o2c_;
}

std::string Comm::getName()
{
    return name_;
}

uint64_t Comm::getDeviceId()
{
    return id_;
}

uint64_t Comm::getConnectedId()
{
    return cId_;
}

void Comm::MarkDataOld()
{
    newDataAvailable_ = false;
}

size_t Comm::getRxBufferCount() const
{
    return rxBuffer_.size();
}

size_t Comm::getTxBufferCount() const
{
    return txBuffer_.size();
}

bool Comm::isNewDataAvailable()
{
    return newDataAvailable_;
}

void Comm::setRenderable(bool render)
{
    renderable_ = render;
}

bool Comm::isRenderable()
{
    return renderable_;
}

void Comm::Connect(uint64_t deviceId)
{
    cId_ = deviceId;
}

void Comm::SendMessage(const std::string& data)
{
    SendMessage(std::vector<uint8_t>(data.begin(), data.end()));
}

void Comm::SendMessage(const std::vector<uint8_t>& data)
{
    if(cId_ > 0)
    {
        auto msg = std::make_shared<CommDataFrame>();
        msg->seq = txSeq_++;
        msg->source = id_;
        msg->destination = cId_;
        msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
        msg->data = data;
        txBuffer_.push_back(msg);
    }
}

std::shared_ptr<CommDataFrame> Comm::ReadMessage()
{
    std::shared_ptr<CommDataFrame> msg {nullptr};
    if(rxBuffer_.size() > 0)
    {
        msg = rxBuffer_[0];
        rxBuffer_.pop_front();
    }
    return msg;
}

void Comm::MessageReceived(std::shared_ptr<CommDataFrame> message)
{
    rxBuffer_.push_back(message);
}

void Comm::ProcessMessages()
{
}

void Comm::AttachToWorld(const Transform& origin)
{
    o2c_ = origin;
}

void Comm::AttachToStatic(StaticEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2c_ = origin;
        attach_ = body;
    }
}

void Comm::AttachToSolid(MovingEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2c_ = origin;
        attach_ = body;
    }
}

void Comm::Update(Scalar dt)
{
    SDL_LockMutex(updateMutex_);
    InternalUpdate(dt);
    SDL_UnlockMutex(updateMutex_);
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
