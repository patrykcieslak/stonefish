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
//  SSS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/06/20.
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/SSS.h"

#include "core/GraphicalSimulationApp.h"
#include "core/DeviceFactory.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLSSS.h"

namespace sf
{

SSS::SSS(const std::string& uniqueName, unsigned int numOfBins, unsigned int numOfLines, Scalar verticalBeamWidthDeg,
         Scalar horizontalBeamWidthDeg, Scalar verticalTiltDeg, Scalar minRange, Scalar maxRange, SonarOutputFormat outputFormat, Scalar frequency)
    : Camera(uniqueName, (numOfBins%2==0 ? numOfBins : numOfBins+1), numOfLines, verticalBeamWidthDeg, frequency)
{
    range_.x = 0.f;
    range_.y = 0.f;
    noise_ = glm::vec2(0.f);
    setRangeMax(maxRange);
    setRangeMin(minRange);
    gain_ = Scalar(1);
    fovV_ = horizontalBeamWidthDeg <= Scalar(0) ? Scalar(1) : (horizontalBeamWidthDeg > Scalar(90) ? Scalar(90) : horizontalBeamWidthDeg);
    tilt_ = verticalTiltDeg < Scalar(0) ? Scalar(0) : (verticalTiltDeg > Scalar(90) ? Scalar(90) : verticalTiltDeg);
    cMap_ = ColorMap::GREEN_BLUE;
    outputFormat_ = outputFormat;
    sonarData_ = nullptr;
    newDataCallback_ = nullptr;
    glSSS_ = nullptr;
}

void SSS::setRangeMin(Scalar r)
{
    range_.x = r < Scalar(0.02) ? 0.02f : (r < Scalar(range_.y) ? (GLfloat)r : range_.x);
}

void SSS::setRangeMax(Scalar r)
{
    range_.y = r > Scalar(range_.x) ? (GLfloat)r : range_.x;
    Scalar pulseTime = (Scalar(2)*range_.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
    if(freq_ <= 0.0 || freq_ > Scalar(1)/pulseTime) // Limit update frequency based on range (physical limit)
        freq_ = Scalar(1)/pulseTime;
}

void SSS::setGain(Scalar g)
{
    gain_ = g > Scalar(0) ? g : Scalar(1);
}

void SSS::setNoise(float multiplicativeStdDev, float additiveStdDev)
{
    if(multiplicativeStdDev >= 0.f)
        noise_.x = multiplicativeStdDev;
    if(additiveStdDev >= 0.f)
        noise_.y = additiveStdDev;
    if(glSSS_ != nullptr)
        glSSS_->setNoise(noise_);
}

void SSS::setDisplaySettings(ColorMap cm)
{
    cMap_ = cm;
}

void* SSS::getImageDataPointer(unsigned int index)
{
    return sonarData_;
}

void SSS::getDisplayResolution(unsigned int& x, unsigned int& y) const
{
    getResolution(x, y); //numOfBins x numOfLines
}

GLubyte* SSS::getDisplayDataPointer()
{
    return displayData_.data();
}

Scalar SSS::getRangeMin() const
{
    return Scalar(range_.x);
}

Scalar SSS::getRangeMax() const
{
    return Scalar(range_.y);
}

Scalar SSS::getGain() const
{
    return gain_;
}

SonarOutputFormat SSS::getOutputFormat() const
{
    return outputFormat_;
}
   
VisionSensorType SSS::getVisionSensorType() const
{
    return VisionSensorType::SSS;
}

OpenGLView* SSS::getOpenGLView() const
{
    return glSSS_;
}

void SSS::InitGraphics(bool& seesParticles)
{
    seesParticles = false;

    // Create sonar
    std::unique_ptr<OpenGLSSS> glSSS = std::make_unique<OpenGLSSS>(
        glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0),
        (GLfloat)fovH_, (GLfloat)fovV_, (GLint)resX_, (GLint)resY_, (GLfloat)tilt_, range_, outputFormat_
    );

    // Set up sonar
    glSSS_ = glSSS.get();
    glSSS_->setNoise(noise_);
    glSSS_->setSonar(this);
    glSSS_->setColorMap(cMap_);
    UpdateTransform();
    glSSS_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glSSS));

    unsigned int w, h;
    getDisplayResolution(w, h);
    displayData_.resize(w*h*3);
}

void SSS::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glSSS_->SetupSonar(eye_, dir_, up_);
}

void SSS::InstallNewDataHandler(std::function<void(SSS*)> callback)
{
    newDataCallback_ = callback;
}

void SSS::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback_ != NULL)
    {
        if(index == 0)
        {
            unsigned int w, h;
            getDisplayResolution(w, h);
            memcpy(displayData_.data(), data, w*h*3);
        }
        else
        {
            sonarData_ = data;
            newDataCallback_(this);
            sonarData_ = NULL;
        }
    }
}

void SSS::InternalUpdate(Scalar dt)
{
    if(glSSS_ != nullptr)
        glSSS_->Update();
}

std::vector<Renderable> SSS::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item1;
        item1.type = RenderableType::SENSOR_LINES; 
        item1.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item1.getDataAsPoints();   
        
        //Create single transducer dummy
        int div = 12;
        GLfloat fovStep = glm::radians(fovH_)/(GLfloat)div;
        //Arcs min
        GLfloat cosVAngle = cosf(glm::radians(fovV_)/2.f) * range_.x;
        GLfloat sinVAngle = sinf(glm::radians(fovV_)/2.f) * range_.x;        
        GLfloat hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Arcs max
        cosVAngle = cosf(glm::radians(fovV_)/2.f) * range_.y;
        sinVAngle = sinf(glm::radians(fovV_)/2.f) * range_.y;        
        hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Ends
        hAngle = -fovStep*(div/2);
        GLfloat zs = cosf(hAngle) * cosVAngle;
        GLfloat xs = sinf(hAngle) * cosVAngle;
        points->push_back(glm::vec3(xs, sinVAngle, zs));
        points->push_back(glm::vec3(xs, -sinVAngle, zs));
        hAngle = fovStep*(div/2);
        GLfloat ze = cosf(hAngle) * cosVAngle;
        GLfloat xe = sinf(hAngle) * cosVAngle;
        points->push_back(glm::vec3(xe, sinVAngle, ze));
        points->push_back(glm::vec3(xe, -sinVAngle, ze));
        //Pyramid
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(xs, sinVAngle, zs));
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(xs, -sinVAngle, zs));
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(xe, sinVAngle, ze));
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(xe, -sinVAngle, ze));

        //Add two transducer dummies
        GLfloat offsetAngle = M_PI_2 - glm::radians(tilt_);
        glm::mat4 views[2];
        views[0] = glm::rotate(-offsetAngle, glm::vec3(0.f,1.f,0.f));
        views[1] = glm::rotate(offsetAngle, glm::vec3(0.f,1.f,0.f));
        item1.model = glMatrixFromTransform(getSensorFrame()) * views[0];
        items.push_back(item1);

        Renderable item2;
        item2.type = item1.type;
        item2.data = item1.data;
        item2.model = glMatrixFromTransform(getSensorFrame()) * views[1];
        items.push_back(item2);
    }
    return items;
}

// Statics

ConstructInfo SSS::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;

    // Specs
    node.optional = false;
    node.attributes.insert({"bins", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"lines", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"horizontal_beam_width", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"vertical_beam_width", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"vertical_tilt", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"output_format", {ConstructInfoValueType::STRING, true}});
    info.nodes.insert({"specs", node});

    // Settings
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"range_min", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"range_max", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"gain", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"settings", node});

    // Noise
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"multiplicative", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"additive", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"noise", node});

    // Display
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"colormap", {ConstructInfoValueType::COLORMAP, true}});
    info.nodes.insert({"display", node});

    return info;
}

std::unique_ptr<SSS> SSS::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // Specs
    int bins = std::get<int>(info.nodes.at("specs").attributes.at("bins").value);
    int lines = std::get<int>(info.nodes.at("specs").attributes.at("lines").value);
    Scalar hFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("horizontal_beam_width").value);
    Scalar vFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("vertical_beam_width").value);
    Scalar tilt = std::get<Scalar>(info.nodes.at("specs").attributes.at("vertical_tilt").value);
    
    SonarOutputFormat outputFormat {SonarOutputFormat::U8};
    ConstructInfoValue& value = info.nodes.at("specs").attributes.at("output_format");
    if (value.valid)
    {
        std::string of = std::get<std::string>(value.value);
        if (of == "uint8")
            outputFormat = SonarOutputFormat::U8;
        else if (of == "uint16")
            outputFormat = SonarOutputFormat::U16;
        else if (of == "uint32")
            outputFormat = SonarOutputFormat::U32;
        else if(of == "float32")
            outputFormat = SonarOutputFormat::F32;
    }

    // Settings
    Scalar rangeMin = std::get<Scalar>(info.nodes.at("settings").attributes.at("range_min").value);
    Scalar rangeMax = std::get<Scalar>(info.nodes.at("settings").attributes.at("range_max").value);
    Scalar gain {1.};
    value = info.nodes.at("settings").attributes.at("gain");
    if (value.valid)
        gain = std::get<Scalar>(value.value);

    // Construct
    std::unique_ptr<SSS> sensor = std::make_unique<SSS>(uniqueName, bins, lines, hFov, vFov, tilt, rangeMin, rangeMax, outputFormat, frequency);
    sensor->setGain(gain);
    
    // Noise
    Scalar multiplicative {0.01};
    Scalar additive {0.02};
    
    value = info.nodes.at("noise").attributes.at("multiplicative");
    if (value.valid)
        multiplicative = std::get<Scalar>(value.value);

    value = info.nodes.at("noise").attributes.at("additive");
    if (value.valid)
        additive = std::get<Scalar>(value.value);

    sensor->setNoise(multiplicative, additive);

    // Display
    value = info.nodes.at("display").attributes.at("colormap");
    if (value.valid)
        sensor->setDisplaySettings(std::get<ColorMap>(value.value));

    return sensor;
}

REGISTER_SENSOR("sss", SSS)

}
