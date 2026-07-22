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
#include "core/DeviceFactory.h"
#include "graphics/OpenGLRealCamera.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

ColorCamera::ColorCamera(const std::string& uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar hFOVDeg, Scalar frequency, 
    Scalar near, Scalar far) : Camera(uniqueName, resolutionX, resolutionY, hFOVDeg, frequency)
{
    depthRange_ = glm::vec2((GLfloat)near, (GLfloat)far);
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
    
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glCamera));
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

// Statics

ConstructInfo ColorCamera::getConstructInfo()
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

    return info;
}

std::unique_ptr<ColorCamera> ColorCamera::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
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

    return std::make_unique<ColorCamera>(uniqueName, resolutionX, resolutionY, hFov, frequency, near, far);
}

REGISTER_SENSOR("camera", ColorCamera)

}
