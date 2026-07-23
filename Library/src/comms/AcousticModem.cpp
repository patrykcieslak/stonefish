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
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "comms/AcousticModem.h"

#include <algorithm>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{
 
AcousticModem::AcousticModem(const std::string& uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange)
    : Comm(uniqueName, deviceId)
{
    btClamp(maxVerticalFOVDeg, Scalar(0), Scalar(360));
    btClamp(minVerticalFOVDeg, Scalar(0), maxVerticalFOVDeg);
    minFov2_ = btRadians(minVerticalFOVDeg/Scalar(2));
    maxFov2_ = btRadians(maxVerticalFOVDeg/Scalar(2));
    range_ = operatingRange <= Scalar(0) ? Scalar(1000) : operatingRange;
    position_ = V0();
    frame_ = std::string("");
    occlusion_ = true;
    addNode(this);
}

AcousticModem::~AcousticModem()
{
    removeNode(this->getDeviceId());
}

bool AcousticModem::isReceptionPossible(Vector3 worldDir, Scalar distance)
{
    //Check if modems are close enough
    if(distance > range_) return false;
        
    //Check if direction is in the FOV of the device
    Vector3 dir = (getDeviceFrame().getBasis().inverse() * worldDir).normalized();
    Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).safeNorm();
    Scalar vAngle = M_PI_2; // When dir.z == 0.0
    if(!btFuzzyZero(dir.getZ()))
        vAngle = atan2(d, -dir.getZ());
    return btFabs(vAngle) >= minFov2_ && btFabs(vAngle) <= maxFov2_;
}

void AcousticModem::setOcclusionTest(bool enabled)
{
    occlusion_ = enabled;
}

bool AcousticModem::getOcclusionTest() const
{
    return occlusion_;
}

void AcousticModem::getPosition(Vector3& pos, std::string& referenceFrame)
{
    pos = position_;
    referenceFrame = frame_;
}

CommType AcousticModem::getType() const
{
    return CommType::ACOUSTIC;
}

void AcousticModem::SendMessage(const std::vector<uint8_t>& data)
{    
    if(getConnectedId() < 0) // Not connected
        return;
    else if(getConnectedId() == 0) // Broadcast
    {
        std::vector<uint64_t> nodeIds = getNodeIds();
        for(size_t i=0; i<nodeIds.size(); ++i)
        {
            if(nodeIds[i] != getDeviceId() && mutualContact(getDeviceId(), nodeIds[i]))
            {
                auto msg = std::make_shared<AcousticDataFrame>();
                msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
                msg->seq = txSeq_++;
                msg->source = getDeviceId();
                msg->destination = nodeIds[i];
                msg->data = data;
                msg->txPosition = getDeviceFrame().getOrigin();
                msg->travelled = Scalar(0);
                txBuffer_.push_back(msg);
            }
        }
    }
    else // Conneted to one receiver
    {
        if(!mutualContact(getDeviceId(), getConnectedId()))
            return;
        
        auto msg = std::make_shared<AcousticDataFrame>();
        msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
        msg->seq = txSeq_++;
        msg->source = getDeviceId();
        msg->destination = getConnectedId();
        msg->data = data;
        msg->txPosition = getDeviceFrame().getOrigin();
        msg->travelled = Scalar(0);
        txBuffer_.push_back(msg);
    }
}

void AcousticModem::ProcessMessages()
{
    std::shared_ptr<AcousticDataFrame> msg;
    std::string ack {"ACK"};
    std::vector<uint8_t> ackData {ack.begin(), ack.end()};
    std::string ping {"PING"};
    std::vector<uint8_t> pingData {ping.begin(), ping.end()};

    for(auto it = rxBuffer_.begin(); it != rxBuffer_.end(); ++it)
    {
        // Send "ACK" message back to sender
        msg = std::static_pointer_cast<AcousticDataFrame>(*it);
        if(msg->data != ackData)
        {
            //timestamp and sequence don't change
            std::shared_ptr<AcousticDataFrame> ackMsg = std::make_shared<AcousticDataFrame>();
            ackMsg->timeStamp = msg->timeStamp;
            ackMsg->seq = msg->seq;
            ackMsg->destination = msg->source;
            ackMsg->source = getDeviceId();
            ackMsg->data = ackData;
            ackMsg->txPosition = getDeviceFrame().getOrigin();
            ackMsg->travelled = msg->travelled;
            txBuffer_.push_back(ackMsg);
        }
    }
    // Remove "ACK" and "PING" messages from the RX buffer
    rxBuffer_.erase(std::remove_if(rxBuffer_.begin(), rxBuffer_.end(),
        [&ackData, &pingData](const std::shared_ptr<CommDataFrame>& msg) {
            return (msg->data == ackData || msg->data == pingData);
        }), rxBuffer_.end());
}

void AcousticModem::InternalUpdate(Scalar dt)
{
    //Propagate messages already sent
    for(auto mIt = propagating_.begin(); mIt != propagating_.end();)
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
            mIt = propagating_.erase(mIt);
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
    if(txBuffer_.size() > 0)
    {
        auto msg = std::static_pointer_cast<AcousticDataFrame>(txBuffer_[0]);
        txBuffer_.pop_front();

        // The message is sent or lost
        if(mutualContact(msg->source, msg->destination))
        {
            propagating_.insert(std::make_pair(msg, msg->txPosition));
        }
    }
}

void AcousticModem::UpdatePosition(Vector3 pos, bool absolute, std::string referenceFrame)
{
    position_ = pos;
    if(absolute)
        frame_ = std::string("");
    else 
        frame_ = referenceFrame;
    newDataAvailable_ = true;
}

std::vector<Renderable> AcousticModem::Render()
{
    std::vector<Renderable> items(0);
    
    //Fov indicator
    Renderable item1;
    item1.model = glMatrixFromTransform(getDeviceFrame());
    item1.type = RenderableType::SENSOR_LINES;
    item1.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item1.getDataAsPoints();

    GLfloat iconSize = 0.25f;
    int div = 24;
    //Upper circle
    if(minFov2_ > Scalar(0))
    {
        GLfloat r = iconSize * glm::sin((GLfloat)minFov2_);
        GLfloat h = iconSize * glm::cos((GLfloat)minFov2_);
        for(int i=0; i<=div; ++i)
        {
            GLfloat angle = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
            points->push_back(glm::vec3(glm::cos(angle)*r, glm::sin(angle)*r, -h));
            if(i > 0 && i < div)
                points->push_back(points->back());
        }
    }
    //Lower circle
    if(maxFov2_ < Scalar(M_PI))
    {
        GLfloat r = iconSize * glm::sin((GLfloat)maxFov2_);
        GLfloat h = iconSize * glm::cos((GLfloat)maxFov2_);
        for(int i=0; i<=div; ++i)
        {
            GLfloat angle = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
            points->push_back(glm::vec3(glm::cos(angle)*r, glm::sin(angle)*r, -h));
            if(i > 0 && i < div)
                points->push_back(points->back());
        }
    }
    //4 bars
    div = 16;
    if(maxFov2_ - minFov2_ > Scalar(0))
    {
        for(int i=0; i<4; ++i)
        {
            GLfloat hangle = (GLfloat)(i * M_PI_2);
            GLfloat x = iconSize * glm::cos(hangle);
            GLfloat y = iconSize * glm::sin(hangle);
            for(int h=0; h<=div; ++h)
            {
                GLfloat angle = (GLfloat)h/(GLfloat)div * (maxFov2_-minFov2_) + minFov2_;
                points->push_back(glm::vec3(glm::sin(angle)*x, glm::sin(angle)*y, -glm::cos(angle)*iconSize));
                if(h == 0 && minFov2_ > Scalar(0))
                {
                    glm::vec3 v = points->back();
                    points->push_back(glm::vec3(0.f,0.f,0.f));
                    points->push_back(v);
                }
                else if(h == div && maxFov2_ < Scalar(M_PI))
                {
                    points->push_back(points->back());
                    points->push_back(glm::vec3(0.f,0.f,0.f));
                }
                else if(h > 0 && h < div)
                    points->push_back(points->back());
            }
        }
    }
    items.push_back(item1);

    //Axes
    Renderable item2;
    item2.type = RenderableType::SENSOR_CS;
    items.push_back(item2);

    //Connected nodes
    Renderable item3;
    item3.type = RenderableType::SENSOR_LINES;
    item3.model = glm::mat4(1.f);
    item3.data = std::make_shared<std::vector<glm::vec3>>();
    points = item3.getDataAsPoints();

    if(getConnectedId() == 0)
    {
        std::vector<uint64_t> nodeIds = getNodeIds();
        for(size_t i=0; i<nodeIds.size(); ++i)
            if(nodeIds[i] != getDeviceId())
            {               
                Transform Tn = getNode(nodeIds[i])->getDeviceFrame();
                points->push_back(glVectorFromVector(getDeviceFrame().getOrigin()));
                points->push_back(glVectorFromVector(Tn.getOrigin()));
            }
    }
    else if(getConnectedId() > 0)
    {
        AcousticModem* cNode = getNode(getConnectedId());
        if(cNode != nullptr)
        {
            points->push_back(glVectorFromVector(getDeviceFrame().getOrigin()));
            points->push_back(glVectorFromVector(cNode->getDeviceFrame().getOrigin()));    
        }
    }
    if(!points->empty())
        items.push_back(item3);

#ifdef DEBUG
    Renderable item4;
    item4.type = RenderableType::SENSOR_POINTS;
    item4.model = glm::mat4(1.f);
    item4.data = std::make_shared<std::vector<glm::vec3>>();
    points = item4.getDataAsPoints();

    for( auto mIt = propagating_.begin(); mIt != propagating_.end(); ++mIt)
    {
        Vector3 mPos = mIt->second;
        points->push_back(glm::vec3((GLfloat)mPos.getX(), (GLfloat)mPos.getY(), (GLfloat)mPos.getZ()));
    }
    items.push_back(item4);
#endif

    return items;
}

// Statics

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

ConstructInfo AcousticModem::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Specs
    node.optional = false;
    node.attributes.insert({"min_vertical_fov", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"max_vertical_fov", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"range", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"specs", node});

    // Connect
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"device_id", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"occlusion_test", {ConstructInfoValueType::BOOL, true}});
    info.nodes.insert({"connect", node});
    
    return info;
}

std::unique_ptr<AcousticModem> AcousticModem::Construct(const std::string& uniqueName, uint64_t deviceId, ConstructInfo& info)
{
    // Required
    Scalar minVerticalFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("min_vertical_fov").value);
    Scalar maxVerticalFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("max_vertical_fov").value);
    Scalar range = std::get<Scalar>(info.nodes.at("specs").attributes.at("range").value);

    // Construct
    std::unique_ptr<AcousticModem> comm = std::make_unique<AcousticModem>(uniqueName, deviceId, minVerticalFov, maxVerticalFov, range);    
    comm->Connect(std::get<int>(info.nodes.at("connect").attributes.at("device_id").value));
    
    // Optional
    bool occlusionTest {true};
    ConstructInfoValue& value = info.nodes.at("connect").attributes.at("occlusion_test");
    if (value.valid)
        occlusionTest = std::get<bool>(value.value);
    comm->setOcclusionTest(occlusionTest);
    
    return comm;
}

REGISTER_COMM("acoustic_modem", AcousticModem)

}