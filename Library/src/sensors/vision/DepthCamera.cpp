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
//  DepthCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 07/05/18.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/DepthCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLDepthCamera.h"

namespace sf
{

DepthCamera::DepthCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar minDepth, Scalar maxDepth, Scalar frequency)
    : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_.x = minDepth < Scalar(0.01) ? 0.01f : (GLfloat)minDepth;
    depthRange_.y = maxDepth > Scalar(0.01) ? (GLfloat)maxDepth : 1.f;
    noiseStdDev_ = 0.f;
    newDataCallback_ = nullptr;
    imageData_ = nullptr;
    glCamera_ = nullptr;
}

void DepthCamera::setNoise(float depthStdDev)
{
    if(depthStdDev >= 0.f && depthStdDev != noiseStdDev_)
    {
        noiseStdDev_ = depthStdDev;
        if(glCamera_ != nullptr)
            glCamera_->setNoise(noiseStdDev_);
    }
}

void* DepthCamera::getImageDataPointer(unsigned int index)
{
    return imageData_;
}

glm::vec2 DepthCamera::getDepthRange() const
{
    return depthRange_;
}
    
VisionSensorType DepthCamera::getVisionSensorType() const
{
    return VisionSensorType::DEPTH_CAMERA;
}

OpenGLView* DepthCamera::getOpenGLView() const
{
    return glCamera_;
}

void DepthCamera::InitGraphics(bool& seesParticles)
{
    seesParticles = false;

    // Create camera
    std::unique_ptr<OpenGLDepthCamera> glCamera =
        std::make_unique<OpenGLDepthCamera>(
            glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX_, resY_, (GLfloat)fovH_,
            depthRange_.x, depthRange_.y, freq_ < Scalar(0)
        );

    // Set up camera
    glCamera_ = glCamera.get();
    glCamera_->setNoise(noiseStdDev_);
    glCamera_->setCamera(this);
    UpdateTransform();
    glCamera_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));
}

void DepthCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera_->SetupCamera(eye_, dir_, up_);
}

void DepthCamera::InstallNewDataHandler(std::function<void(DepthCamera*)> callback)
{
    newDataCallback_ = callback;
}

void DepthCamera::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback_ != nullptr)
    {
        imageData_ = (GLfloat*)data;
        newDataCallback_(this);
        imageData_ = nullptr;
    }
}

void DepthCamera::InternalUpdate(Scalar dt)
{
    glCamera_->Update();
}

}
