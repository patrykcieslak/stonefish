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
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/EventBasedCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLEventBasedCamera.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

EventBasedCamera::EventBasedCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, 
    float Cp, float Cm, uint32_t Tref, Scalar frequency, Scalar minDistance, Scalar maxDistance) 
    : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    C_.x = glm::abs(Cp);
    C_.y = glm::abs(Cm);
    Tr_ = Tref;
    lastEventCount_ = 0;
    newDataCallback_ = nullptr;
    imageData_ = nullptr;
    glCamera_ = nullptr;
}

void EventBasedCamera::setNoise(float sigmaCp, float sigmaCm)
{
    sigmaC_.x = glm::abs(sigmaCp);
    sigmaC_.y = glm::abs(sigmaCm);
    if(glCamera_ != nullptr)
        glCamera_->setNoise(sigmaC_);
}

void* EventBasedCamera::getImageDataPointer(unsigned int index)
{
    return imageData_;
}

unsigned int EventBasedCamera::getLastEventCount() const
{
    return lastEventCount_;
}

VisionSensorType EventBasedCamera::getVisionSensorType() const
{
    return VisionSensorType::EVENT_BASED_CAMERA;
}

OpenGLView* EventBasedCamera::getOpenGLView() const
{
    return glCamera_;
}
    
void EventBasedCamera::InitGraphics(bool& seesParticles)
{
    seesParticles = true;

    // Create camera
    std::unique_ptr<OpenGLEventBasedCamera> glCamera = std::make_unique<OpenGLEventBasedCamera>(
        glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX_, resY_, (GLfloat)fovH_, 
        depthRange_, C_, Tr_, freq_ < Scalar(0)
    );

    // Setup camera
    glCamera_ = glCamera.get();
    glCamera_->setNoise(sigmaC_);
    glCamera_->setCamera(this);
    UpdateTransform();
    glCamera_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));
}

void EventBasedCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera_->SetupCamera(eye_, dir_, up_);
}

void EventBasedCamera::InstallNewDataHandler(std::function<void(EventBasedCamera*)> callback)
{
    newDataCallback_ = callback;
}

void EventBasedCamera::NewDataReady(void* data, unsigned int index)
{
    lastEventCount_ = index;

#ifdef DEBUG
    if(lastEventCount_ > 0)
    {
        GLint* data_ = (GLint*)data;
        int firstTime = INT32_MAX;
        int lastTime = 0;
        for(unsigned int i = 0; i < lastEventCount_; ++i)
        {
            if(abs(data_[i*2+1]) > lastTime)
                lastTime = abs(data_[i*2+1]);
            if(abs(data_[i*2+1]) < firstTime)
                firstTime = abs(data_[i*2+1]);   
        }
        printf("First event time: %d, Last event time: %d\n", firstTime, lastTime);
    }
#endif

    if(newDataCallback_ != nullptr)
    {
        imageData_ = (GLint*)data;
        newDataCallback_(this);
        imageData_ = nullptr;
    }
}

void EventBasedCamera::InternalUpdate(Scalar dt)
{
    glCamera_->Update();
}

}
