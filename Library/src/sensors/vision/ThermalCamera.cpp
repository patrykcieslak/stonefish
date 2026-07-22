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
#include "core/DeviceFactory.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLThermalCamera.h"

namespace sf
{

ThermalCamera::ThermalCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar minTemp, Scalar maxTemp,
    Scalar frequency, Scalar near, Scalar far) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)near, (GLfloat)far);
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

// Static

ConstructInfo ThermalCamera::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;

    // Specs
    node.optional = false;
    node.attributes.insert({"resolution_x", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"resolution_y", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"horizontal_fov", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"temperature_min", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"temperature_max", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"specs", node});

    // Rendering
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"minimum_distance", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"maximum_distance", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"rendering", node});

    // Noise
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"temperature", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"noise", node});

    // Display
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"colormap", {ConstructInfoValueType::COLORMAP, true}});
    node.attributes.insert({"temperature_min", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"temperature_max", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"display", node});

    return info;
}

std::unique_ptr<ThermalCamera> ThermalCamera::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // Specs
    int resolutionX = std::get<int>(info.nodes.at("specs").attributes.at("resolution_x").value);
    int resolutionY = std::get<int>(info.nodes.at("specs").attributes.at("resolution_y").value);
    Scalar hFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("horizontal_fov").value);
    Scalar temperatureMin = std::get<Scalar>(info.nodes.at("specs").attributes.at("temperature_min").value);
    Scalar temperatureMax = std::get<Scalar>(info.nodes.at("specs").attributes.at("temperature_max").value);
    
    // Rendering (optional)
    Scalar near (STD_NEAR_PLANE_DISTANCE);
    Scalar far (STD_FAR_PLANE_DISTANCE);

    ConstructInfoValue& value = info.nodes.at("rendering").attributes.at("near");
    if (value.valid)
        near = std::get<Scalar>(value.value);

    value = info.nodes.at("rendering").attributes.at("far");
    if (value.valid)
        far = std::get<Scalar>(value.value);

    std::unique_ptr<ThermalCamera> sensor = std::make_unique<ThermalCamera>(uniqueName, resolutionX, resolutionY,
        hFov, temperatureMin, temperatureMax, frequency, near, far);

    // Noise
    value = info.nodes.at("noise").attributes.at("temperature");
    if (value.valid)
        sensor->setNoise(std::get<Scalar>(value.value));

    // Display
    ColorMap cMap (ColorMap::JET);

    value = info.nodes.at("display").attributes.at("colormap");
    if (value.valid)
        cMap = std::get<ColorMap>(value.value);

    value = info.nodes.at("display").attributes.at("temperature_min");
    if (value.valid)
        temperatureMin = std::get<Scalar>(value.value);

    value = info.nodes.at("display").attributes.at("temperature_max");
    if (value.valid)
        temperatureMax = std::get<Scalar>(value.value);

    sensor->setDisplaySettings(cMap, temperatureMin, temperatureMax);

    return sensor;
}

REGISTER_SENSOR("thermal_camera", ThermalCamera)

}
