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
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/ColorCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLRealCamera.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

ColorCamera::ColorCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, Scalar frequency, 
    Scalar minDistance, Scalar maxDistance) : Camera(uniqueName, resolutionX, resolutionY, horizFOVDeg, frequency)
{
    depthRange = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    newDataCallback = NULL;
    imageData = NULL;
}

ColorCamera::~ColorCamera()
{
    glCamera = NULL;
}
    
void ColorCamera::setExposureCompensation(Scalar comp)
{
    if(glCamera != NULL)
        glCamera->setExposureCompensation((GLfloat)comp);
}
    
Scalar ColorCamera::getExposureCompensation()
{
    if(glCamera != NULL)
        return (Scalar)glCamera->getExposureCompensation();
    else
        return Scalar(0);
}

void* ColorCamera::getImageDataPointer(unsigned int index)
{
    return imageData;
}

VisionSensorType ColorCamera::getVisionSensorType()
{
    return VisionSensorType::SENSOR_COLOR_CAMERA;
}
    
void ColorCamera::InitGraphics()
{
    glCamera = new OpenGLRealCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, depthRange);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);
}

void ColorCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void ColorCamera::InstallNewDataHandler(std::function<void(ColorCamera*)> callback)
{
    newDataCallback = callback;
}

void ColorCamera::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback != NULL)
    {
        imageData = (GLubyte*)data;
        newDataCallback(this);
        imageData = NULL;
    }
}

void ColorCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
