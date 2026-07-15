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
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/ThermalCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLThermalCamera.h"

namespace sf
{

ThermalCamera::ThermalCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar minTemp, Scalar maxTemp,
    Scalar frequency, Scalar minDistance, Scalar maxDistance) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)minDistance, (GLfloat)maxDistance);
    noiseStdDev_ = 0.f;
    measurementRange_ = glm::vec2((GLfloat)minTemp, (GLfloat)maxTemp);
    displayRange_ = measurementRange_;
    colorMap_ = ColorMap::JET;
    newDataCallback_ = nullptr;
    temperatureData_ = nullptr;
    glCamera_ = nullptr;
}

void ThermalCamera::setNoise(GLfloat tempStdDev)
{
    if(tempStdDev >= 0.f && tempStdDev != noiseStdDev_)
    {
        noiseStdDev_ = tempStdDev;
        if(glCamera_ != nullptr)
            glCamera_->setNoise(noiseStdDev_);
    }
}

void ThermalCamera:: setDisplaySettings(ColorMap cm, Scalar minTemp, Scalar maxTemp)
{
    colorMap_ = cm;

    if(minTemp > maxTemp)
        displayRange_ = measurementRange_;
    else
    {
        glm::vec2 tempRange = glm::vec2((GLfloat)minTemp, (GLfloat)maxTemp);
        displayRange_ = glm::clamp(tempRange, measurementRange_.x, measurementRange_.y);
    }
        
    if(glCamera_ != nullptr)
    {
        glCamera_->setColorMap(colorMap_);
        glCamera_->setDisplayRange(displayRange_);
    }
}

void* ThermalCamera::getImageDataPointer(unsigned int index)
{
    return temperatureData_;
}

GLubyte* ThermalCamera::getDisplayDataPointer()
{
    return displayData_.data();
}

VisionSensorType ThermalCamera::getVisionSensorType() const
{
    return VisionSensorType::THERMAL_CAMERA;
}

OpenGLView* ThermalCamera::getOpenGLView() const
{
    return glCamera_;
}

void ThermalCamera::InitGraphics(bool& seesParticles)
{
    seesParticles = false;

    // Create camera
    std::unique_ptr<OpenGLThermalCamera> glCamera = std::make_unique<OpenGLThermalCamera>(
        glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX_, resY_, (GLfloat)fovH_, measurementRange_, depthRange_, freq_ < Scalar(0)
    );

    // Set up camera
    glCamera_ = glCamera.get();
    glCamera_->setNoise(noiseStdDev_);
    glCamera_->setColorMap(colorMap_);
    glCamera_->setDisplayRange(displayRange_);
    glCamera_->setCamera(this);
    UpdateTransform();
    glCamera_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));

    unsigned int w, h;
    getResolution(w, h);
    displayData_.resize(w*h*3);
}

void ThermalCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera_->SetupCamera(eye_, dir_, up_);
}

void ThermalCamera::InstallNewDataHandler(std::function<void(ThermalCamera*)> callback)
{
    newDataCallback_ = callback;
}

void ThermalCamera::NewDataReady(void* data, unsigned int index)
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
            temperatureData_ = (GLfloat*)data;
            newDataCallback_(this);
            temperatureData_ = nullptr;
        }
    }
}

void ThermalCamera::InternalUpdate(Scalar dt)
{
    glCamera_->Update();
}

}
