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
//  MSIS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/07/20.
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/MSIS.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLMSIS.h"

namespace sf
{

MSIS::MSIS(const std::string& uniqueName, Scalar stepAngleDeg, unsigned int numOfBins, Scalar horizontalBeamWidthDeg, Scalar verticalBeamWidthDeg,
           Scalar minRotationDeg, Scalar maxRotationDeg, Scalar minRange, Scalar maxRange, ColorMap cm, SonarOutputFormat outputFormat, Scalar frequency)
    : Camera(uniqueName, (unsigned int)ceil(Scalar(360)/stepAngleDeg), numOfBins, horizontalBeamWidthDeg, frequency)
{
    range_.x = 0.f;
    range_.y = 0.f;
    noise_ = glm::vec2(0.f);
    fullRotation_ = false;
    stepSize_ = btRadians(btScalar(360)/ceil(Scalar(360)/stepAngleDeg)); //Corrected step angle in radians
    setRotationLimits(minRotationDeg, maxRotationDeg);
    setRangeMax(maxRange);
    setRangeMin(minRange);
    setGain(1);
    fovV_ = verticalBeamWidthDeg <= Scalar(0) ? Scalar(20) : (verticalBeamWidthDeg > Scalar(179) ? Scalar(179) : verticalBeamWidthDeg);
    cMap_ = cm;
    outputFormat_ = outputFormat;
    sonarData_ = nullptr;
    newDataCallback_ = nullptr;
    glMSIS_ = nullptr;
}

void MSIS::setRotationLimits(Scalar l1Deg, Scalar l2Deg)
{
    if(l1Deg == l2Deg) //Same
        return;

    if(l1Deg > l2Deg) //Swap
    {
        Scalar tmp = l1Deg;
        l1Deg = l2Deg;
        l2Deg = tmp;
    }

    //Clamp and convert to number of steps
    btClamp(l1Deg, Scalar(-180), Scalar(180));
    btClamp(l2Deg, Scalar(-180), Scalar(180));
    roi_.x = (GLint)round(btRadians(l1Deg)/stepSize_);
    roi_.y = (GLint)round(btRadians(l2Deg)/stepSize_);
    fullRotation_ = (roi_.x == -(int)resX_/2) && (roi_.y == (int)resX_/2);
    if(roi_.y == (int)resX_/2)
        --roi_.y;
    currentStep_ = roi_.x;
    cw_ = true;
}

void MSIS::setRangeMin(Scalar r)
{
    range_.x = r < Scalar(0.02) ? 0.02f : (r < Scalar(range_.y) ? (GLfloat)r : range_.x);
}

void MSIS::setRangeMax(Scalar r)
{
    range_.y = r > Scalar(range_.x) ? (GLfloat)r : range_.x;
    Scalar pulseTime = (Scalar(2)*range_.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
    if(freq_ <= 0.0 || freq_ > Scalar(1)/pulseTime) // Limit update frequency based on range (physical limit)
        freq_ = Scalar(1)/pulseTime;
}

void MSIS::setGain(Scalar g)
{
    gain_ = g > Scalar(0) ? g : Scalar(1);
}

void MSIS::setNoise(float multiplicativeStdDev, float additiveStdDev)
{
    if(multiplicativeStdDev >= 0.f)
        noise_.x = multiplicativeStdDev;
    if(additiveStdDev >= 0.f)
        noise_.y = additiveStdDev;
    if(glMSIS_ != nullptr)
        glMSIS_->setNoise(noise_);
}

void* MSIS::getImageDataPointer(unsigned int index)
{
    return sonarData_;
}

void MSIS::getDisplayResolution(unsigned int& x, unsigned int& y) const
{
    getResolution(x, y);
    x = y; //numOfBins x numOfBins
}

GLubyte* MSIS::getDisplayDataPointer()
{
    return displayData_.data();
}

void MSIS::getRotationLimits(Scalar& l1Deg, Scalar& l2Deg) const
{
    l1Deg = btDegrees(Scalar(roi_.x) * stepSize_);
    l2Deg = btDegrees(Scalar(roi_.y) * stepSize_);
}

Scalar MSIS::getRotationStepAngle() const
{
    return btDegrees(stepSize_);
}

int MSIS::getCurrentRotationStep() const
{
    return currentStep_;
}

GLuint MSIS::getCurrentBeamIndex() const
{
    return (GLuint)(currentStep_ + (GLint)(resX_/2));
}

Scalar MSIS::getRangeMin() const
{
    return Scalar(range_.x);
}

Scalar MSIS::getRangeMax() const
{
    return Scalar(range_.y);
}

Scalar MSIS::getGain() const
{
    return gain_;
}

SonarOutputFormat MSIS::getOutputFormat() const
{
    return outputFormat_;
}
    
VisionSensorType MSIS::getVisionSensorType() const
{
    return VisionSensorType::MSIS;
}

OpenGLView* MSIS::getOpenGLView() const
{
    return glMSIS_;
}

void MSIS::InitGraphics(bool& seesParticles)
{
    seesParticles = false;

    // Create sonar
    std::unique_ptr<OpenGLMSIS> glMSIS = std::make_unique<OpenGLMSIS>(
        glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0),
        (GLfloat)fovH_, (GLfloat)fovV_, (GLint)resX_, (GLint)resY_, range_, outputFormat_
    );

    // Set up sonar
    glMSIS_ = glMSIS.get();
    glMSIS_->setNoise(noise_);
    glMSIS_->setSonar(this);
    glMSIS_->setColorMap(cMap_);
    UpdateTransform();
    glMSIS_->UpdateTransform();
    InternalUpdate(0);

    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(std::move(glMSIS));

    unsigned int w, h;
    getDisplayResolution(w, h);
    displayData_.resize(w*h*3);
}

void MSIS::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glMSIS_->SetupSonar(eye_, dir_, up_);
}

void MSIS::InstallNewDataHandler(std::function<void(MSIS*)> callback)
{
    newDataCallback_ = callback;
}

void MSIS::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback_ != nullptr)
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
            sonarData_ = nullptr;
        }
    }

    if(index == 1)
    {
        currentStep_ += cw_ ? 1 : -1;
        
        if(fullRotation_)
        {
            if(cw_ && currentStep_ > roi_.y)
                currentStep_ = roi_.x;
            else if(!cw_ && currentStep_ < roi_.x)
                currentStep_ = roi_.y;
        }
        else
        {
            if(cw_ && currentStep_ == roi_.y)
                cw_ = false;
            else if(!cw_ && currentStep_ == roi_.x)
                cw_ = true;
        }
    }
}

void MSIS::InternalUpdate(Scalar dt)
{
    if(glMSIS_ != nullptr)
        glMSIS_->Update();
}

std::vector<Renderable> MSIS::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.type = RenderableType::SENSOR_LINES;    
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item.getDataAsPoints();
        
        //Create sonar dummy
        int div = 24;
        Scalar l1Deg, l2Deg;
        getRotationLimits(l1Deg, l2Deg);
        GLfloat fovStep = fullRotation_ ? 2.f*M_PI/(GLfloat)div : glm::radians(l2Deg-l1Deg)/(GLfloat)div;
        //Arcs min
        GLfloat cosVAngle = cosf(glm::radians(fovV_)/2.f) * range_.x;
        GLfloat sinVAngle = sinf(glm::radians(fovV_)/2.f) * range_.x;
        GLfloat hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = glm::radians(l1Deg);
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
        hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                points->push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Current beam position
        hAngle = currentStep_ * stepSize_;
        GLfloat zc = cosf(hAngle) * cosVAngle;
        GLfloat xc = sinf(hAngle) * cosVAngle;
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(xc, sinVAngle, zc));
        points->push_back(glm::vec3(xc, sinVAngle, zc));
        points->push_back(glm::vec3(xc, -sinVAngle, zc));
        points->push_back(glm::vec3(xc, -sinVAngle, zc));
        points->push_back(glm::vec3(0,0,0));
        
        if(!fullRotation_)
        {
            //Ends
            hAngle = glm::radians(l1Deg);
            GLfloat zs = cosf(hAngle) * cosVAngle;
            GLfloat xs = sinf(hAngle) * cosVAngle;
            points->push_back(glm::vec3(xs, sinVAngle, zs));
            points->push_back(glm::vec3(xs, -sinVAngle, zs));
            hAngle = glm::radians(l2Deg);
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
        }

        items.push_back(item);
    }
    return items;
}

}
