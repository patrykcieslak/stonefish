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
//  ThermalCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 26/05/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/ThermalCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLThermalCamera.h"

namespace sf
{

ThermalCamera::ThermalCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar minTemp, Scalar maxTemp,
    Scalar frequency, Scalar minDistance, Scalar maxDistance) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    noiseStdDev = 0.f;
    measurementRange = glm::vec2((GLfloat)minTemp, (GLfloat)maxTemp);
    displayRange = measurementRange;
    colorMap = ColorMap::JET;
    newDataCallback = nullptr;
    temperatureData = nullptr;
    displayData = nullptr;
    glCamera = nullptr;
}

ThermalCamera::~ThermalCamera()
{
    glCamera = nullptr;
}

void ThermalCamera::setNoise(GLfloat tempStdDev)
{
    if(tempStdDev >= 0.f && tempStdDev != noiseStdDev)
    {
        noiseStdDev = tempStdDev;
        if(glCamera != nullptr)
            glCamera->setNoise(noiseStdDev);
    }
}

void ThermalCamera:: setDisplaySettings(ColorMap cm, Scalar minTemp, Scalar maxTemp)
{
    colorMap = cm;

    if(minTemp > maxTemp)
        displayRange = measurementRange;
    else
    {
        glm::vec2 tempRange = glm::vec2((GLfloat)minTemp, (GLfloat)maxTemp);
        displayRange = glm::clamp(tempRange, measurementRange.x, measurementRange.y);
    }
        
    if(glCamera != nullptr)
    {
        glCamera->setColorMap(colorMap);
        glCamera->setDisplayRange(displayRange);
    }
}

void* ThermalCamera::getImageDataPointer(unsigned int index)
{
    return temperatureData;
}

GLubyte* ThermalCamera::getDisplayDataPointer()
{
    return displayData;
}

VisionSensorType ThermalCamera::getVisionSensorType() const
{
    return VisionSensorType::THERMAL_CAMERA;
}

void ThermalCamera::InitGraphics()
{
    glCamera = new OpenGLThermalCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, measurementRange, depthRange, freq < Scalar(0));
    glCamera->setNoise(noiseStdDev);
    glCamera->setColorMap(colorMap);
    glCamera->setDisplayRange(displayRange);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);

    unsigned int w, h;
    getResolution(w, h);
    displayData = new GLubyte[w*h*3];
}

void ThermalCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void ThermalCamera::InstallNewDataHandler(std::function<void(ThermalCamera*)> callback)
{
    newDataCallback = callback;
}

void ThermalCamera::NewDataReady(void* data, unsigned int index)
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
            temperatureData = (GLfloat*)data;
            newDataCallback(this);
            temperatureData = nullptr;
        }
    }
}

void ThermalCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
