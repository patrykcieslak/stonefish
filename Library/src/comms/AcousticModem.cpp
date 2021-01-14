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
//  AcousticModem.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 26/02/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "comms/AcousticModem.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{
 
//Static
std::map<uint64_t, AcousticModem*> AcousticModem::nodes; 

void AcousticModem::addNode(AcousticModem* node)
{
    if(node->getDeviceId() == 0)
    {
        cError("Modem device ID=0 not allowed!");
        return;
    }
        
    if(nodes.find(node->getDeviceId()) != nodes.end())
        cError("Modem node with ID=%d already exists!", node->getDeviceId());
    else
        nodes[node->getDeviceId()] = node;
}

void AcousticModem::removeNode(uint64_t deviceId)
{
    if(deviceId == 0)
        return;
        
    std::map<uint64_t, AcousticModem*>::iterator it = nodes.find(deviceId);
    if(it != nodes.end())
        nodes.erase(it);
}

AcousticModem* AcousticModem::getNode(uint64_t deviceId)
{
    if(deviceId == 0)
        return NULL;
    
    try
    {
        return nodes.at(deviceId);
    }
    catch(const std::out_of_range& oor)
    {
        return NULL;
    }
}   

bool AcousticModem::mutualContact(uint64_t device1Id, uint64_t device2Id)
{
    AcousticModem* node1 = getNode(device1Id);
    AcousticModem* node2 = getNode(device2Id);
    
    if(node1 == NULL || node2 == NULL)
        return false;
        
    Vector3 pos1 = node1->getDeviceFrame().getOrigin();
    Vector3 pos2 = node2->getDeviceFrame().getOrigin();
    Vector3 dir = pos2-pos1;
    Scalar distance = dir.length();
    
    if(!node1->isReceptionPossible(dir, distance) || !node2->isReceptionPossible(-dir, distance))
        return false;
        
    if(node1->getOcclusionTest() || node2->getOcclusionTest())
    {
        btCollisionWorld::ClosestRayResultCallback closest(pos1, pos2);
        closest.m_collisionFilterGroup = MASK_DYNAMIC;
        closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(pos1, pos2, closest);
        return !closest.hasHit();
    }
    else
        return true;
}

//Member 
AcousticModem::AcousticModem(std::string uniqueName, uint64_t deviceId, 
                             Scalar horizontalFOVDeg, Scalar verticalFOVDeg, Scalar operatingRange) : Comm(uniqueName, deviceId)
{
    hFov2 = horizontalFOVDeg <= Scalar(0) || horizontalFOVDeg > Scalar(360) ? Scalar(M_PI) : horizontalFOVDeg/Scalar(180*2)*Scalar(M_PI);
    vFov2 = verticalFOVDeg <= Scalar(0) || verticalFOVDeg > Scalar(360) ? Scalar(M_PI) : verticalFOVDeg/Scalar(180*2)*Scalar(M_PI);
    range = operatingRange <= Scalar(0) ? Scalar(1000) : operatingRange;
    position = V0();
    frame = std::string("");
    occlusion = true;
    addNode(this);
}

AcousticModem::~AcousticModem()
{
    removeNode(this->getDeviceId());
}

bool AcousticModem::isReceptionPossible(Vector3 worldDir, Scalar distance)
{
    //Check if modems are close enough
    if(distance > range) return false;
        
    //Check if direction is in the FOV of the device
    Vector3 dir = (getDeviceFrame().inverse() * worldDir).normalized();
    Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).length();
    Scalar vAngle = atan2(d, dir.getZ());
    Scalar hAngle = atan2(dir.getY(), dir.getX());
    return fabs(hAngle) <= hFov2 && fabs(vAngle) <= vFov2;
}

void AcousticModem::setOcclusionTest(bool enabled)
{
    occlusion = enabled;
}

bool AcousticModem::getOcclusionTest() const
{
    return occlusion;
}

void AcousticModem::getPosition(Vector3& pos, std::string& referenceFrame)
{
    pos = position;
    referenceFrame = frame;
}

CommType AcousticModem::getType() const
{
    return CommType::ACOUSTIC;
}

void AcousticModem::SendMessage(std::string data)
{    
    if(!mutualContact(getDeviceId(), getConnectedId()))
       return;
    
    AcousticDataFrame* msg = new AcousticDataFrame();
    msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
    msg->seq = txSeq++;
    msg->source = getDeviceId();
    msg->destination = getConnectedId();
    msg->data = data;
    msg->txPosition = getDeviceFrame().getOrigin();
    msg->travelled = Scalar(0);
    txBuffer.push_back(msg);
}

void AcousticModem::ProcessMessages()
{
    AcousticDataFrame* msg;
    while((msg = (AcousticDataFrame*)ReadMessage()) != nullptr)
    {
        //Different responses to messages should be implemented here
        if(msg->data != "ACK")
        {
            //timestamp and sequence don't change
            msg->destination = msg->source;
            msg->source = getDeviceId();
            msg->data = "ACK";
            msg->txPosition = getDeviceFrame().getOrigin();
            txBuffer.push_back(msg);
        }
        else
        {
            delete msg;
        }
    }
}

void AcousticModem::InternalUpdate(Scalar dt)
{
    //Propagate messages already sent
    std::map<AcousticDataFrame*, Vector3>::iterator mIt;
    for(mIt = propagating.begin(); mIt != propagating.end(); )
    {
        AcousticModem* dest = getNode(mIt->first->destination);
        Vector3 dO = dest->getDeviceFrame().getOrigin();
        Vector3 sO = mIt->second;
        Vector3 dir = dO - sO;
        Scalar d = dir.length();
        
        if(d <= SOUND_VELOCITY_WATER*dt) //Message reached?
        {
            mIt->first->travelled += d;
            dest->MessageReceived(mIt->first);
            mIt = propagating.erase(mIt);
        }
        else //Advance pulse
        {
            dir /= d; //Normalize direction
            d = SOUND_VELOCITY_WATER * dt;
            mIt->second += dir * d;
            mIt->first->travelled += d;
            ++mIt;
        }
    }
    
    //Send first message from the tx buffer
    if(txBuffer.size() > 0)
    {
        AcousticDataFrame* msg = (AcousticDataFrame*)txBuffer[0];
        if(mutualContact(msg->source, msg->destination))
            propagating[msg] = msg->txPosition;
        else
            delete msg;
            
        txBuffer.pop_front();
    }
}

void AcousticModem::UpdatePosition(Vector3 pos, bool absolute, std::string referenceFrame)
{
    position = pos;
    if(absolute)
        frame = std::string("");
    else 
        frame = referenceFrame;
    newDataAvailable = true;
}

std::vector<Renderable> AcousticModem::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.type = RenderableType::SENSOR_CS;
    item.model = glMatrixFromTransform(getDeviceFrame());
    items.push_back(item);

#ifdef DEBUG
    item.type = RenderableType::SENSOR_POINTS;
    item.model = glm::mat4(1.f);
    std::map<AcousticDataFrame*, Vector3>::iterator mIt;
    for(mIt = propagating.begin(); mIt != propagating.end(); ++mIt)
    {
        Vector3 mPos = mIt->second;
        item.points.push_back(glm::vec3((GLfloat)mPos.getX(), (GLfloat)mPos.getY(), (GLfloat)mPos.getZ()));
    }
    items.push_back(item);
#endif

    return items;
}

}