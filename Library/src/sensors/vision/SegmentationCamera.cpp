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
//  SegmentationCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 14/03/25.
//  Copyright (c) 2025-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/SegmentationCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLSegmentationCamera.h"

namespace sf
{

SegmentationCamera::SegmentationCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar frequency, 
    Scalar minDistance, Scalar maxDistance) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    newDataCallback_ = nullptr;
    segmentationData_= nullptr;
    glCamera_ = nullptr;
}

void* SegmentationCamera::getImageDataPointer(unsigned int index)
{
    return segmentationData_;
}

GLubyte* SegmentationCamera::getDisplayDataPointer()
{
    return displayData_.data();
}

VisionSensorType SegmentationCamera::getVisionSensorType() const
{
    return VisionSensorType::SEGMENTATION_CAMERA;
}

OpenGLView* SegmentationCamera::getOpenGLView() const
{
    return glCamera_;
}

void SegmentationCamera::InitGraphics(bool& seesParticles)
{
    seesParticles = true;

    // Create camera
    std::unique_ptr<OpenGLSegmentationCamera> glCamera = std::make_unique<OpenGLSegmentationCamera>(
        glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX_, resY_, (GLfloat)fovH_, depthRange_, freq_ < Scalar(0)
    );

    // Set up camera
    glCamera_ = glCamera.get();
    glCamera_->setCamera(this);
    UpdateTransform();
    glCamera_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));

    unsigned int w, h;
    getResolution(w, h);
    displayData_.resize(w*h*3);
}

void SegmentationCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera_->SetupCamera(eye_, dir_, up_);
}

void SegmentationCamera::InstallNewDataHandler(std::function<void(SegmentationCamera*)> callback)
{
    newDataCallback_ = callback;
}

void SegmentationCamera::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback_ != nullptr)
    {
        if(index == 0)
        {
            unsigned int w, h;
            getResolution(w, h);
            memcpy(displayData_.data(), data, w*h*3);
        }
        else
        {
            segmentationData_ = (GLushort*)data;
            newDataCallback_(this);
            segmentationData_ = nullptr;
        }
    }
}

void SegmentationCamera::InternalUpdate(Scalar dt)
{
    glCamera_->Update();
}

}
