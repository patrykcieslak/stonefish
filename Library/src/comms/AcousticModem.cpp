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
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
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
        return nullptr;
    
    try
    {
        return nodes.at(deviceId);
    }
    catch(const std::out_of_range& oor)
    {
        return nullptr;
    }
} 

std::vector<uint64_t> AcousticModem::getNodeIds()
{
    std::vector<uint64_t> ids;
    for(auto it=nodes.begin(); it != nodes.end(); ++it)
        ids.push_back(it->first);
    return ids;
}

bool AcousticModem::mutualContact(uint64_t device1Id, uint64_t device2Id)
{
    AcousticModem* node1 = getNode(device1Id);
    AcousticModem* node2 = getNode(device2Id);
    
    if(node1 == nullptr || node2 == nullptr)
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
AcousticModem::AcousticModem(std::string uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange)
                                : Comm(uniqueName, deviceId)
{
    btClamp(maxVerticalFOVDeg, Scalar(0), Scalar(360));
    btClamp(minVerticalFOVDeg, Scalar(0), maxVerticalFOVDeg);
    minFov2 = btRadians(minVerticalFOVDeg/Scalar(2));
    maxFov2 = btRadians(maxVerticalFOVDeg/Scalar(2));
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
    Vector3 dir = (getDeviceFrame().getBasis().inverse() * worldDir).normalized();
    Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).safeNorm();
    Scalar vAngle = M_PI_2; // When dir.z == 0.0
    if(!btFuzzyZero(dir.getZ()))
        vAngle = atan2(d, -dir.getZ());
    return btFabs(vAngle) >= minFov2 && btFabs(vAngle) <= maxFov2;
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
    if(getConnectedId() < 0)
        return;
    else if(getConnectedId() == 0)
    {
        std::vector<uint64_t> nodeIds = getNodeIds();
        for(size_t i=0; i<nodeIds.size(); ++i)
            if(nodeIds[i] != getDeviceId() && mutualContact(getDeviceId(), nodeIds[i]))
            {
                AcousticDataFrame* msg = new AcousticDataFrame();
                msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
                msg->seq = txSeq++;
                msg->source = getDeviceId();
                msg->destination = nodeIds[i];
                msg->data = data;
                msg->txPosition = getDeviceFrame().getOrigin();
                msg->travelled = Scalar(0);
                txBuffer.push_back(msg);
            }
    }
    else
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
    
    //Fov indicator
    Renderable item;
    item.model = glMatrixFromTransform(getDeviceFrame());
    item.type = RenderableType::SENSOR_LINES;
    GLfloat iconSize = 0.25f;
    int div = 24;
    //Upper circle
    if(minFov2 > Scalar(0))
    {
        GLfloat r = iconSize * glm::sin((GLfloat)minFov2);
        GLfloat h = iconSize * glm::cos((GLfloat)minFov2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat angle = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
            item.points.push_back(glm::vec3(glm::cos(angle)*r, glm::sin(angle)*r, -h));
            if(i > 0 && i < div)
                item.points.push_back(item.points.back());
        }
    }
    //Lower circle
    if(maxFov2 < Scalar(M_PI))
    {
        GLfloat r = iconSize * glm::sin((GLfloat)maxFov2);
        GLfloat h = iconSize * glm::cos((GLfloat)maxFov2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat angle = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
            item.points.push_back(glm::vec3(glm::cos(angle)*r, glm::sin(angle)*r, -h));
            if(i > 0 && i < div)
                item.points.push_back(item.points.back());
        }
    }
    //4 bars
    div = 16;
    if(maxFov2 - minFov2 > Scalar(0))
    {
        for(int i=0; i<4; ++i)
        {
            GLfloat hangle = (GLfloat)(i * M_PI_2);
            GLfloat x = iconSize * glm::cos(hangle);
            GLfloat y = iconSize * glm::sin(hangle);
            for(int h=0; h<=div; ++h)
            {
                GLfloat angle = (GLfloat)h/(GLfloat)div * (maxFov2-minFov2) + minFov2;
                item.points.push_back(glm::vec3(glm::sin(angle)*x, glm::sin(angle)*y, -glm::cos(angle)*iconSize));
                if(h == 0 && minFov2 > Scalar(0))
                {
                    glm::vec3 v = item.points.back();
                    item.points.push_back(glm::vec3(0.f,0.f,0.f));
                    item.points.push_back(v);
                }
                else if(h == div && maxFov2 < Scalar(M_PI))
                {
                    item.points.push_back(item.points.back());
                    item.points.push_back(glm::vec3(0.f,0.f,0.f));
                }
                else if(h > 0 && h < div)
                    item.points.push_back(item.points.back());
            }
        }
    }
    items.push_back(item);

    //Axes
    item.type = RenderableType::SENSOR_CS;
    items.push_back(item);

    //Connected nodes
    item.type = RenderableType::SENSOR_LINES;
    item.model = glm::mat4(1.f);
    item.points.clear();
    if(getConnectedId() == 0)
    {
        std::vector<uint64_t> nodeIds = getNodeIds();
        for(size_t i=0; i<nodeIds.size(); ++i)
            if(nodeIds[i] != getDeviceId())
            {               
                Transform Tn = getNode(nodeIds[i])->getDeviceFrame();
                item.points.push_back(glVectorFromVector(getDeviceFrame().getOrigin()));
                item.points.push_back(glVectorFromVector(Tn.getOrigin()));
            }
    }
    else if(getConnectedId() > 0)
    {
        AcousticModem* cNode = getNode(getConnectedId());
        if(cNode != nullptr)
        {
            item.points.push_back(glVectorFromVector(getDeviceFrame().getOrigin()));
            item.points.push_back(glVectorFromVector(cNode->getDeviceFrame().getOrigin()));    
        }
    }
    if(!item.points.empty())
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