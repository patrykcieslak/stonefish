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
//  Light.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2023 Patryk Cieslak. All rights reserved.
//

#include "comms/VLC.h"
#include "graphics/OpenGLLight.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"


namespace sf
{
//Static
std::map<uint64_t, VLC*> VLC::nodes; 

// Helper function to compute size of data_mission
size_t getDataMissionSize(const CommDataFrame::DataType& data_mission) {
    return std::visit([](auto&& value) -> size_t {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double>) {
            return sizeof(T);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return value.size();
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<double>>) {
            return value.size() * sizeof(typename T::value_type);
        } else if constexpr (std::is_same_v<T, PointCloud>) {
            return value.points.size() * sizeof(double);
        }
        return 0;
    }, data_mission);
}

VLC::VLC(std::string uniqueName, uint64_t deviceId, Scalar r, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg,  Scalar cs): Comm(uniqueName, deviceId) {
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use lights in console simulation! Use graphical simulation if possible.");
    
    R=0.0005;
    //Fi = lum < Scalar(0) ? Scalar(0) : lum;
    range=r;
    comm_speed=cs;
    active=false;
    data=NULL;
    water_type=0.0;
}

    
CommType VLC::getType() const
{
    return CommType::VLC;
}


void VLC::SwitchOff(){
     for(int i=0;i<15;i++){
         lights[i]->getGLLight()->SwitchOff();
     }
}

void VLC::SwitchOn(){
     for(int i=0;i<15;i++){
         lights[i]->getGLLight()->SwitchOn();
     }
}

bool VLC::isActive(){
     return active;
}
        

void VLC::enable(bool t){
     active=t;
}

bool VLC::mutualContact(uint64_t device1Id, uint64_t device2Id)
{
    VLC* node1 = getNode(device1Id);
    VLC* node2 = getNode(device2Id);
    
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

VLC* VLC::getNode(uint64_t deviceId)
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

std::vector<uint64_t> VLC::getNodeIds()
{
    std::vector<uint64_t> ids;
    for(auto it=nodes.begin(); it != nodes.end(); ++it)
        ids.push_back(it->first);
    return ids;
}

void VLC::InternalUpdate(Scalar dt)
{
    
    //Send first message from the tx buffer
    if(txBuffer.size() > 0)
    {
        CommDataFrame* msg = (CommDataFrame*)txBuffer[0];
        if(mutualContact(msg->source, msg->destination)){
            VLC* dest = getNode(msg->destination);
            dest->MessageReceived(msg);
        }
        else
            delete msg;
            
        txBuffer.pop_front();
    }
}


void VLC::SendMessage(std::string data, const CommDataFrame::DataType& data_mission)
{
    float waterQuality = 0; // Assume this returns a value between 0 and 1

    // Compute size of the data_mission and check against allowed size based on water quality
    size_t dataSize = getDataMissionSize(data_mission);
    size_t maxSizeAllowed = static_cast<size_t>(1000 * (1.0f - waterQuality)); // Example: max size decreases as waterQuality increases

    if (dataSize > maxSizeAllowed) {
        //std::cout << "Message dropped due to excessive data_mission size based on water quality.\n";
        return;
    }

    // Message might be dropped or truncated based on water quality
    if (waterQuality > 0.8f) {
        if (rand() / (float)RAND_MAX < waterQuality) {
            //std::cout << "Message dropped due to poor water quality.\n";
            return;
        }
    } else if (waterQuality > 0.5f) {
        float truncationFactor = 1.0f - waterQuality; // The worse the quality, the smaller the message
        size_t truncateLength = static_cast<size_t>(data.size() * truncationFactor);

        // Truncate message
        data = data.substr(0, truncateLength);
        //std::cout << "Message truncated due to water quality.\n";
    }

    if(getConnectedId() < 0)
        return;
    else if(getConnectedId() == 0)
    {
        std::vector<uint64_t> nodeIds = getNodeIds();
        for(size_t i=0; i<nodeIds.size(); ++i)
        {
            if(nodeIds[i] != getDeviceId() && mutualContact(getDeviceId(), nodeIds[i]))
            {
                CommDataFrame* msg = new CommDataFrame();
                msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
                msg->seq = txSeq++;
                msg->source = getDeviceId();
                msg->destination = nodeIds[i];
                msg->data = data;
                msg->data_mission = data_mission;
                txBuffer.push_back(msg);
            }
        }
    }
    else
    {
        if(!mutualContact(getDeviceId(), getConnectedId()))
            return;
        
        CommDataFrame* msg = new CommDataFrame();
        msg->timeStamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
        msg->seq = txSeq++;
        msg->source = getDeviceId();
        msg->destination = getConnectedId();
        msg->data = data;
        msg->data_mission = data_mission;
        txBuffer.push_back(msg);
    }
}

void VLC::ProcessMessages()
{
    CommDataFrame* msg;
    while((msg = (CommDataFrame*)ReadMessage()) != nullptr)
    {
        //Different responses to messages should be implemented here
        if(msg->data != "ACK")
        {
            //timestamp and sequence don't change
            msg->destination = msg->source;
            msg->source = getDeviceId();
            msg->data = "ACK";
            data=msg;
            txBuffer.push_back(msg);
        }
        else
        {
            delete msg;
        }
    }
    
}

bool VLC::getOcclusionTest() const
{
    return occlusion;
}


bool VLC::isReceptionPossible(Vector3 worldDir, Scalar distance)
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
    
std::vector<Renderable> VLC::Render()
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
        VLC* cNode = getNode(getConnectedId());
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
    std::map<CommDataFrame*, Vector3>::iterator mIt;
    for(mIt = propagating.begin(); mIt != propagating.end(); ++mIt)
    {
        Vector3 mPos = mIt->second;
        item.points.push_back(glm::vec3((GLfloat)mPos.getX(), (GLfloat)mPos.getY(), (GLfloat)mPos.getZ()));
    }
    items.push_back(item);
#endif

    return items;
}

std::vector<Light*> VLC::getLights(){
    return lights;
}

void VLC::addLight(Light* l){
    return lights.push_back(l);
}


Scalar VLC::getRange(){
       return range;
}
        
void VLC::setRange(Scalar r){
     range=r;
}
        
Scalar VLC::getCommSpeed(){
       return comm_speed;
}
        
void VLC::setCommSpeed(Scalar cs){
     comm_speed=cs;
}

CommDataFrame* VLC::getData(){
     return data;
}

void VLC::setWaterType(Scalar t){
     water_type=t;
}

}
