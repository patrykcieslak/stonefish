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
//  OpticalModem.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 14/05/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include "comms/OpticalModem.h"

#include <random>
#include <chrono>   
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{
 
//Static
std::map<uint64_t, OpticalModem*> OpticalModem::nodes; 

void OpticalModem::addNode(OpticalModem* node)
{
    if(node->getDeviceId() == 0)
    {
        cError("Modem device ID=0 not allowed!");
        return;
    }
        
    if(nodes.find(node->getDeviceId()) != nodes.end())
    {
        cError("Modem node with ID=%d already exists!", node->getDeviceId());
    }
    else
    {
        nodes[node->getDeviceId()] = node;
    }
}

void OpticalModem::removeNode(uint64_t deviceId)
{
    if(deviceId == 0)
        return;
        
    std::map<uint64_t, OpticalModem*>::iterator it = nodes.find(deviceId);
    if(it != nodes.end())
    {
        nodes.erase(it);
    }
}

OpticalModem* OpticalModem::getNode(uint64_t deviceId)
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

std::vector<uint64_t> OpticalModem::getNodeIds()
{
    std::vector<uint64_t> ids;
    for(auto it=nodes.begin(); it != nodes.end(); ++it)
    {
        ids.push_back(it->first);
    }
    return ids;
}

// Function to introduce random errors in a byte stream
// based on a link_quality_factor (0.0 to 1.0).
// A link_quality_factor of 1.0 means no errors (perfect quality).
// A link_quality_factor of 0.0 means maximum errors (every byte potentially changed).
std::vector<uint8_t> OpticalModem::introduceErrors(
    const std::vector<uint8_t>& data, Scalar linkQuality) 
{
    if (data.empty()) 
    {
        return {};
    }

    btClamp(linkQuality, Scalar(0), Scalar(1));

    if (linkQuality == Scalar(1))
    {
        return data;
    }

    // Calculate the actual probability of an error occurring for each byte
    double errorProbability = Scalar(1) - linkQuality;
    
    std::vector<uint8_t> erroredData {data};
    
    // Modern C++ random number generation
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed); // Mersenne Twister engine

    // Distribution for deciding IF an error occurs at a position
    std::uniform_real_distribution<Scalar> errorProbabilityDistribution(Scalar(0), Scalar(1));

    // Distribution for picking a random byte value (0-255)
    std::uniform_int_distribution<uint8_t> byteDistribution(0, 255);

    for (size_t i = 0; i < erroredData.size(); ++i) 
    {
        // Check if an error should be introduced at this position
        if (errorProbabilityDistribution(generator) < errorProbability) 
        {
            uint8_t originalByte = erroredData[i];
            uint8_t newByte;

            // If error probability is 1.0 we ideally want to ensure the byte actually changes.
            if (errorProbability == Scalar(1)) 
            {
                do 
                {
                    newByte = static_cast<unsigned char>(byteDistribution(generator));
                } 
                while (newByte == originalByte); // Keep trying until it's different
            } 
            else 
            {
                // For other probabilities, a single random selection is usually fine.
                // The chance of picking the same byte is 1/256.
                newByte = static_cast<unsigned char>(byteDistribution(generator));
            }
            erroredData[i] = newByte;
        }
    }
    return erroredData;
}

//Member 
OpticalModem::OpticalModem(std::string uniqueName, uint64_t deviceId, Scalar fovDeg, Scalar operatingRange, Scalar ambientLightSensitivity)
                                : Comm(uniqueName, deviceId)
{
    maxRange = operatingRange <= Scalar(0) ? Scalar(100) : operatingRange;
    btClamp(fovDeg, Scalar(0), Scalar(360));
    fov = btRadians(fovDeg);
    ambientLightSens = btClamped(ambientLightSensitivity, Scalar(0), Scalar(1));
    
    trueRange = maxRange;
    receptionQuality = Scalar(1);

    addNode(this);
}

OpticalModem::~OpticalModem()
{
    removeNode(this->getDeviceId());
}

bool OpticalModem::isReceptionPossible(Vector3 worldDir, Scalar distance)
{
    // Check if modems are close enough
    if(distance > trueRange) 
    {
        return false;
    }
        
    // Check if direction is in the FOV of the device
    Vector3 dir = (getDeviceFrame().getBasis().inverse() * worldDir).safeNormalize();
    Scalar d {Vector3(dir.getX(), dir.getY(), Scalar(0)).safeNorm()};
    Scalar vAngle {M_PI_2}; // When dir.z == 0.0
    if(!btFuzzyZero(dir.getZ()))
    {
        vAngle = btAtan2(d, dir.getZ());
    }
    bool possible = btFabs(vAngle) < fov/Scalar(2);
    return possible;
}

Scalar OpticalModem::getReceptionQuality() const
{
    return receptionQuality;
}

CommType OpticalModem::getType() const
{
    return CommType::OPTICAL;
}

void OpticalModem::MessageReceived(std::shared_ptr<CommDataFrame> message)
{
    if(receptionQuality > Scalar(0))
    {
        message->data = introduceErrors(message->data, receptionQuality);
        Comm::MessageReceived(message);
    }
}

void OpticalModem::InternalUpdate(Scalar dt)
{
    // Check if connected to something
    if(getConnectedId() <= 0) 
    {
        receptionQuality = Scalar(0);
        return;
    }

    // Find devices
    OpticalModem* connectedNode = getNode(getConnectedId());
    if(connectedNode == nullptr)
    {
        receptionQuality = Scalar(0);
        return;
    }

    Vector3 posRX {getDeviceFrame().getOrigin()};
    Vector3 posTX {connectedNode->getDeviceFrame().getOrigin()};
    
    // Both devices have to be in the same medium
    bool underwater {false};
    bool receptionPossible {true};

    Ocean* ocean = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(ocean != nullptr)
    {
        Scalar depthRX {ocean->GetDepth(posRX)};
        Scalar depthTX {ocean->GetDepth(posTX)};

        if (depthRX < Scalar(0) && depthTX < Scalar(0)) // In Air
        {
            underwater = false;
            receptionPossible = true;
        }
        else if (depthRX >= Scalar(0) && depthTX >= Scalar(0)) // In water
        {
            underwater = true;
            receptionPossible = true;
        }
        else // Not possible (different mediums)
        {
            receptionPossible = false;
        }           
    }

    // Update true operating range
    if (underwater)
    {
        Scalar turbidity = ocean->getWaterType(); // 0.0 (clear) to 1.0 (very turbid)
        trueRange = maxRange * (Scalar(1) - turbidity);
    }
    else 
    {
        trueRange = maxRange;
    }

    // Check if devices see each other
    if (receptionPossible) 
    {
        Vector3 dir {posTX-posRX};
        Scalar distance {dir.safeNorm()};
    
        receptionPossible = isReceptionPossible(dir, distance);
        
        // Check if there are obstacles between the devices 
        if (receptionPossible)
        {
            btCollisionWorld::ClosestRayResultCallback closest(posRX, posTX);
            closest.m_collisionFilterGroup = MASK_DYNAMIC;
            closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
            SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(posRX, posTX, closest);
            if(closest.hasHit())
            {
                receptionPossible = false;
                receptionQuality = Scalar(0);
            }
            else
            {
                // Exponentially decay the reception quality with distance
                receptionQuality = Scalar(1) - btExp(-Scalar(10)*(trueRange - distance)/trueRange);
            }
        }
    }

    // Calculate the reception quality
    if (receptionPossible)
    {
        // Estimate light intensity factor based on the sun position
        Atmosphere* atm = SimulationApp::getApp()->getSimulationManager()->getAtmosphere();
        if (underwater)
        {
            Scalar az, alt;
            atm->GetSunPosition(az, alt);
            btClamp(alt, Scalar(0), Scalar(90));
            Scalar lightIntensity {Scalar(1) - btCos(btRadians(alt))};

            Scalar directionalFactor {  
                (getDeviceFrame().getBasis().getColumn(2).dot(Vector3(0,0,-1)) // 1 if facing up, -1 when facing down
                + Scalar(1)) // Offset to (0,2) range
                * Scalar(0.5) // Scale to (0,1) range
            };
            Scalar depthRX {ocean->GetDepth(posRX)};
            Scalar turbidity = ocean->getWaterType(); // 0.0 (clear) to 1.0 (very turbid)   
            receptionQuality -= ambientLightSens * lightIntensity * directionalFactor * btExp(-turbidity * depthRX);
        }
        else
        {
            Vector3 sunDir = atm->GetSunDirection();
            Scalar lightIntensity {
                (getDeviceFrame().getBasis().getColumn(2).dot(-sunDir) // 1 if facing sun, -1 when facing opposite
                + Scalar(1)) // Offset to (0,2) range
                * Scalar(0.5) // Scale to (0,1) range
            };
            receptionQuality -= ambientLightSens * lightIntensity;
        }
    }
    else
    {
        receptionQuality = Scalar(0);
    }

    // Send all messages from the TX buffer
    for(size_t i=0; i<txBuffer.size(); ++i)
    {
        connectedNode->MessageReceived(txBuffer[i]);
    }
    txBuffer.clear();
}

std::vector<Renderable> OpticalModem::Render()
{
    std::vector<Renderable> items(0);
    
    //Fov indicator
    Renderable item;
    item.model = glMatrixFromTransform(getDeviceFrame());
    item.type = RenderableType::SENSOR_LINES;
    GLfloat iconSize = 0.25f;
    unsigned int div = 24;
    
    GLfloat r = iconSize * tanf((GLfloat)fov/2.f);    
    for(unsigned int i=0; i<div; ++i)
    {
        GLfloat angle1 = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
        GLfloat angle2 = (GLfloat)(i+1)/(GLfloat)div * 2.f * M_PI;
        item.points.push_back(glm::vec3(r * cosf(angle1), r * sinf(angle1), iconSize));
        item.points.push_back(glm::vec3(r * cosf(angle2), r * sinf(angle2), iconSize));
    }
        
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(r, 0, iconSize));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-r, 0, iconSize));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0, r, iconSize));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0, -r, iconSize));
    items.push_back(item);

    //Axes
    item.type = RenderableType::SENSOR_CS;
    items.push_back(item);

    //Connected nodes
    item.type = RenderableType::SENSOR_LINES;
    item.model = glm::mat4(1.f);
    item.points.clear();
    if(getConnectedId() > 0)
    {
        OpticalModem* cNode = getNode(getConnectedId());
        if(cNode != nullptr)
        {
            item.points.push_back(glVectorFromVector(getDeviceFrame().getOrigin()));
            item.points.push_back(glVectorFromVector(cNode->getDeviceFrame().getOrigin()));    
        }
    }
    if(!item.points.empty())
        items.push_back(item);

    return items;
}

}