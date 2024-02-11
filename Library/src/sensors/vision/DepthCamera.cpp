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
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/DepthCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLDepthCamera.h"

namespace sf
{

DepthCamera::DepthCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizontalFOVDeg, Scalar minDepth, Scalar maxDepth, Scalar frequency)
    : Camera(uniqueName, resolutionX, resolutionY, horizontalFOVDeg, frequency)
{
    depthRange.x = minDepth < Scalar(0.01) ? 0.01f : (GLfloat)minDepth;
    depthRange.y = maxDepth > Scalar(0.01) ? (GLfloat)maxDepth : 1.f;
    noiseStdDev = 0.f;
    newDataCallback = nullptr;
    imageData = nullptr;
    glCamera = nullptr;
}

DepthCamera::~DepthCamera()
{
    glCamera = nullptr;
}

void DepthCamera::setNoise(float depthStdDev)
{
    if(depthStdDev >= 0.f && depthStdDev != noiseStdDev)
    {
        noiseStdDev = depthStdDev;
        if(glCamera != nullptr)
            glCamera->setNoise(noiseStdDev);
    }
}

void* DepthCamera::getImageDataPointer(unsigned int index)
{
    return imageData;
}

glm::vec2 DepthCamera::getDepthRange()
{
    return depthRange;
}
    
VisionSensorType DepthCamera::getVisionSensorType()
{
    return VisionSensorType::DEPTH_CAMERA;
}

void DepthCamera::InitGraphics()
{
    glCamera = new OpenGLDepthCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, depthRange.x, depthRange.y, freq < Scalar(0));
    glCamera->setNoise(noiseStdDev);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);
}

void DepthCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void DepthCamera::InstallNewDataHandler(std::function<void(DepthCamera*)> callback)
{
    newDataCallback = callback;
}

void DepthCamera::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback != nullptr)
    {
        imageData = (GLfloat*)data;
        newDataCallback(this);
        imageData = nullptr;
    }
}

void DepthCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
