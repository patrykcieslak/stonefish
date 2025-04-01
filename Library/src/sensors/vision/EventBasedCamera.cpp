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
//  EventBasedCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/3/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/EventBasedCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLEventBasedCamera.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

EventBasedCamera::EventBasedCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, 
    float Cp, float Cm, uint32_t Tref, Scalar frequency, Scalar minDistance, Scalar maxDistance) 
    : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    C.x = glm::abs(Cp);
    C.y = glm::abs(Cm);
    Tr = Tref;
    lastEventCount = 0;
    newDataCallback = nullptr;
    imageData = nullptr;
    glCamera = nullptr;
}

EventBasedCamera::~EventBasedCamera()
{
    glCamera = nullptr;
}

void EventBasedCamera::setNoise(float sigmaCp, float sigmaCm)
{
    sigmaC.x = glm::abs(sigmaCp);
    sigmaC.y = glm::abs(sigmaCm);
    if(glCamera != nullptr)
        glCamera->setNoise(sigmaC);
}

void* EventBasedCamera::getImageDataPointer(unsigned int index)
{
    return imageData;
}

unsigned int EventBasedCamera::getLastEventCount() const
{
    return lastEventCount;
}

VisionSensorType EventBasedCamera::getVisionSensorType() const
{
    return VisionSensorType::EVENT_BASED_CAMERA;
}

OpenGLView* EventBasedCamera::getOpenGLView() const
{
    return glCamera;
}
    
void EventBasedCamera::InitGraphics()
{
    glCamera = new OpenGLEventBasedCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, 
                                    depthRange, C, Tr, freq < Scalar(0));
    glCamera->setNoise(sigmaC);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);
}

void EventBasedCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void EventBasedCamera::InstallNewDataHandler(std::function<void(EventBasedCamera*)> callback)
{
    newDataCallback = callback;
}

void EventBasedCamera::NewDataReady(void* data, unsigned int index)
{
    lastEventCount = index;

#ifdef DEBUG
    if(lastEventCount > 0)
    {
        GLint* data_ = (GLint*)data;
        int firstTime = INT32_MAX;
        int lastTime = 0;
        for(unsigned int i = 0; i < lastEventCount; ++i)
        {
            if(abs(data_[i*2+1]) > lastTime)
                lastTime = abs(data_[i*2+1]);
            if(abs(data_[i*2+1]) < firstTime)
                firstTime = abs(data_[i*2+1]);   
        }
        printf("First event time: %d, Last event time: %d\n", firstTime, lastTime);
    }
#endif

    if(newDataCallback != nullptr)
    {
        imageData = (GLint*)data;
        newDataCallback(this);
        imageData = nullptr;
    }
}

void EventBasedCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
