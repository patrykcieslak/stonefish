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
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/OpticalFlowCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "core/DeviceFactory.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLOpticalFlowCamera.h"

namespace sf
{

OpticalFlowCamera::OpticalFlowCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar frequency, 
    Scalar near, Scalar far) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)near, (GLfloat)far);
    noiseStdDev_ = glm::vec2(0.f);
    displayMaxVelocity_ = resolutionX/2.f;
    newDataCallback_ = nullptr;
    flowData_ = nullptr;
    glCamera_ = nullptr;
}

void OpticalFlowCamera::setNoise(float velocityXStdDev, float velocityYStdDev)
{
    noiseStdDev_.x = velocityXStdDev > 0.f ? velocityXStdDev : 0.f;
    noiseStdDev_.y = velocityYStdDev > 0.f ? velocityYStdDev : 0.f;
    if(glCamera_ != nullptr)
        glCamera_->setNoise(noiseStdDev_);
}

void OpticalFlowCamera::setDisplaySettings(GLfloat maxVelocity)
{
    displayMaxVelocity_ = glm::abs(maxVelocity);
    if(glCamera_ != nullptr)
        glCamera_->setMaxVelocity(displayMaxVelocity_);
}

void* OpticalFlowCamera::getImageDataPointer(unsigned int index)
{
    return flowData_;
}

GLubyte* OpticalFlowCamera::getDisplayDataPointer()
{
    return displayData_.data();
}

VisionSensorType OpticalFlowCamera::getVisionSensorType() const
{
    return VisionSensorType::OPTICAL_FLOW_CAMERA;
}

OpenGLView* OpticalFlowCamera::getOpenGLView() const
{
    return glCamera_;
}

void OpticalFlowCamera::InitGraphics(bool& seesParticles)
{
    seesParticles = false;

    // Create camera
    std::unique_ptr<OpenGLOpticalFlowCamera> glCamera = std::make_unique<OpenGLOpticalFlowCamera>(
        glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX_, resY_, (GLfloat)fovH_, depthRange_, freq_ < Scalar(0)
    );

    // Set up camera
    glCamera_ = glCamera.get();
    glCamera_->setNoise(noiseStdDev_);
    glCamera_->setMaxVelocity(displayMaxVelocity_);
    glCamera_->setCamera(this);
    UpdateTransform();
    glCamera_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));

    unsigned int w, h;
    getResolution(w, h);
    displayData_.resize(w*h*3);
}

void OpticalFlowCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera_->SetupCamera(eye_, dir_, up_);
}

void OpticalFlowCamera::InstallNewDataHandler(std::function<void(OpticalFlowCamera*)> callback)
{
    newDataCallback_ = callback;
}

void OpticalFlowCamera::NewDataReady(void* data, unsigned int index)
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
            flowData_ = (GLfloat*)data;
            newDataCallback_(this);
            flowData_ = nullptr;
        }
    }
}

void OpticalFlowCamera::InternalUpdate(Scalar dt)
{
    glCamera_->Update();
}

// Statics

ConstructInfo OpticalFlowCamera::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;

    // Specs
    node.optional = false;
    node.attributes.insert({"resolution_x", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"resolution_y", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"horizontal_fov", {ConstructInfoValueType::SCALAR, false}});
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
    node.attributes.insert({"velocity_x", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"velocity_y", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"noise", node});

    // Display
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"velocity_max", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"display", node});

    return info;
}

std::unique_ptr<OpticalFlowCamera> OpticalFlowCamera::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // Specs
    int resolutionX = std::get<int>(info.nodes.at("specs").attributes.at("resolution_x").value);
    int resolutionY = std::get<int>(info.nodes.at("specs").attributes.at("resolution_y").value);
    Scalar hFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("horizontal_fov").value);

    // Rendering (optional)
    Scalar near (STD_NEAR_PLANE_DISTANCE);
    Scalar far (STD_FAR_PLANE_DISTANCE);

    ConstructInfoValue& value = info.nodes.at("rendering").attributes.at("near");
    if (value.valid)
        near = std::get<Scalar>(value.value);

    value = info.nodes.at("rendering").attributes.at("far");
    if (value.valid)
        far = std::get<Scalar>(value.value);

    std::unique_ptr<OpticalFlowCamera> sensor = std::make_unique<OpticalFlowCamera>(uniqueName, resolutionX, resolutionY, hFov, frequency, near, far);

    // Noise
    Scalar velX (0.);
    Scalar velY (0.);
    
    value = info.nodes.at("noise").attributes.at("velocity_x");
    if (value.valid)
        velX = std::get<Scalar>(value.value);
    
    value = info.nodes.at("noise").attributes.at("velocity_y");
    if (value.valid)
        velY = std::get<Scalar>(value.value);

    sensor->setNoise(velX, velY);

    // Display
    value = info.nodes.at("display").attributes.at("velocity_max");
    if (value.valid)
        sensor->setDisplaySettings(std::get<Scalar>(value.value));

    return sensor;
}

REGISTER_SENSOR("optical_flow", OpticalFlowCamera)

}
