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
//  ColorCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/5/18.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/ColorCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLRealCamera.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

ColorCamera::ColorCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar frequency, 
    Scalar minDistance, Scalar maxDistance) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    newDataCallback_ = nullptr;
    imageData_ = nullptr;
    glCamera_ = nullptr;
}
    
void ColorCamera::setExposureCompensation(Scalar comp)
{
    if(glCamera_ != nullptr)
        glCamera_->setExposureCompensation((GLfloat)comp);
}
    
Scalar ColorCamera::getExposureCompensation() const
{
    if(glCamera_ != nullptr)
        return (Scalar)glCamera_->getExposureCompensation();
    else
        return Scalar(0);
}

void* ColorCamera::getImageDataPointer(unsigned int index)
{
    return imageData_;
}

VisionSensorType ColorCamera::getVisionSensorType() const
{
    return VisionSensorType::COLOR_CAMERA;
}

OpenGLView* ColorCamera::getOpenGLView() const
{
    return glCamera_;
}

void ColorCamera::InitGraphics(bool& seesParticles)
{
    seesParticles = true;
    
    // Create camera
    std::unique_ptr<OpenGLRealCamera> glCamera = 
        std::make_unique<OpenGLRealCamera>(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX_, resY_, (GLfloat)fovH_, depthRange_, freq_ < Scalar(0));
    
    // Set up camera
    glCamera_ = glCamera.get();
    glCamera_->setCamera(this);
    UpdateTransform();
    glCamera_->UpdateTransform();
    InternalUpdate(0);
    
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));
}

void ColorCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera_->SetupCamera(eye_, dir_, up_);
}

void ColorCamera::InstallNewDataHandler(std::function<void(ColorCamera*)> callback)
{
    newDataCallback_ = callback;
}

void ColorCamera::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback_ != nullptr)
    {
        imageData_ = (GLubyte*)data;
        newDataCallback_(this);
        imageData_ = nullptr;
    }
}

void ColorCamera::InternalUpdate(Scalar dt)
{
    glCamera_->Update();
}

}
