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
//  OpticalFlowCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 08/02/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/OpticalFlowCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLOpticalFlowCamera.h"

namespace sf
{

OpticalFlowCamera::OpticalFlowCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar frequency, 
    Scalar minDistance, Scalar maxDistance) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    noiseStdDev = glm::vec2(0.f);
    displayMaxVelocity = resolutionX/2.f;
    newDataCallback = nullptr;
    flowData = nullptr;
    displayData = nullptr;
    glCamera = nullptr;
}

OpticalFlowCamera::~OpticalFlowCamera()
{
    glCamera = nullptr;
}

void OpticalFlowCamera::setNoise(float velocityXStdDev, float velocityYStdDev)
{
    noiseStdDev.x = velocityXStdDev > 0.f ? velocityXStdDev : 0.f;
    noiseStdDev.y = velocityYStdDev > 0.f ? velocityYStdDev : 0.f;
    if(glCamera != nullptr)
        glCamera->setNoise(noiseStdDev);
}

void OpticalFlowCamera::setDisplaySettings(GLfloat maxVelocity)
{
    displayMaxVelocity = glm::abs(maxVelocity);
    if(glCamera != nullptr)
        glCamera->setMaxVelocity(displayMaxVelocity);
}

void* OpticalFlowCamera::getImageDataPointer(unsigned int index)
{
    return flowData;
}

GLubyte* OpticalFlowCamera::getDisplayDataPointer()
{
    return displayData;
}

VisionSensorType OpticalFlowCamera::getVisionSensorType() const
{
    return VisionSensorType::OPTICAL_FLOW_CAMERA;
}

OpenGLView* OpticalFlowCamera::getOpenGLView() const
{
    return glCamera;
}

void OpticalFlowCamera::InitGraphics()
{
    glCamera = new OpenGLOpticalFlowCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, depthRange, freq < Scalar(0));
    glCamera->setNoise(noiseStdDev);
    glCamera->setMaxVelocity(displayMaxVelocity);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);

    unsigned int w, h;
    getResolution(w, h);
    displayData = new GLubyte[w*h*3];
}

void OpticalFlowCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void OpticalFlowCamera::InstallNewDataHandler(std::function<void(OpticalFlowCamera*)> callback)
{
    newDataCallback = callback;
}

void OpticalFlowCamera::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback != nullptr)
    {
        if(index == 0)
        {
            unsigned int w, h;
            getResolution(w, h);
            memcpy(displayData, data, w*h*3);
        }
        else
        {
            flowData = (GLfloat*)data;
            newDataCallback(this);
            flowData = nullptr;
        }
    }
}

void OpticalFlowCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
