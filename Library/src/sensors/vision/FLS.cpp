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
//  FLS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/FLS.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLFLS.h"

namespace sf
{

FLS::FLS(std::string uniqueName, unsigned int numOfBeams, unsigned int numOfBins, Scalar horizontalFOVDeg, 
    Scalar verticalFOVDeg, Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency)
    : Camera(uniqueName, numOfBeams, numOfBins, horizontalFOVDeg, frequency)
{
    range.x = 0.f;
    range.y = 0.f;
    noise = glm::vec2(0.f);
    setRangeMax(maxRange);
    setRangeMin(minRange);
    gain = Scalar(1);
    fovV = verticalFOVDeg <= Scalar(0) ? Scalar(20) : (verticalFOVDeg > Scalar(179) ? Scalar(179) : verticalFOVDeg);
    cMap = cm;
    sonarData = NULL;
    displayData = NULL;
    newDataCallback = NULL;
    glFLS = nullptr;
}

FLS::~FLS()
{
    if(displayData != NULL) delete [] displayData;
    glFLS = nullptr;
}

void FLS::setRangeMin(Scalar r)
{
    range.x = r < Scalar(0.02) ? 0.02f : (r < Scalar(range.y) ? (GLfloat)r : range.x);
}

void FLS::setRangeMax(Scalar r)
{
    range.y = r > Scalar(range.x) ? (GLfloat)r : range.x;
    Scalar pulseTime = (Scalar(2)*range.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
    setUpdateFrequency(Scalar(1)/pulseTime);
}

void FLS::setGain(Scalar g)
{
    gain = g > Scalar(0) ? g : Scalar(1);
}

void FLS::setNoise(float multiplicativeStdDev, float additiveStdDev)
{
    if(multiplicativeStdDev >= 0.f)
        noise.x = multiplicativeStdDev;
    if(additiveStdDev >= 0.f)
        noise.y = additiveStdDev;
    if(glFLS != nullptr)
        glFLS->setNoise(noise);
}

void* FLS::getImageDataPointer(unsigned int index)
{
    return sonarData;
}

void FLS::getDisplayResolution(unsigned int& x, unsigned int& y)
{
    getResolution(x, y);
    GLfloat hFactor = sinf(glm::radians((float)fovH)/2.f);
    x = (int)ceilf(2.f*hFactor*y);
}

GLubyte* FLS::getDisplayDataPointer()
{
    return displayData;
}

Scalar FLS::getRangeMin() const
{
    return Scalar(range.x);
}

Scalar FLS::getRangeMax() const
{
    return Scalar(range.y);
}

Scalar FLS::getGain() const
{
    return gain;
}
    
VisionSensorType FLS::getVisionSensorType()
{
    return VisionSensorType::FLS;
}

void FLS::InitGraphics()
{
    glFLS = new OpenGLFLS(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 
                          (GLfloat)fovH, (GLfloat)fovV, (GLint)resX, (GLint)resY, range);
    glFLS->setNoise(noise);
    glFLS->setSonar(this);
    glFLS->setColorMap(cMap);
    UpdateTransform();
    glFLS->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glFLS);

    unsigned int w, h;
    getDisplayResolution(w, h);
    displayData = new GLubyte[w*h*3];
}

void FLS::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glFLS->SetupSonar(eye_, dir_, up_);
}

void FLS::InstallNewDataHandler(std::function<void(FLS*)> callback)
{
    newDataCallback = callback;
}

void FLS::NewDataReady(void* data, unsigned int index)
{
    if(newDataCallback != NULL)
    {
        if(index == 0)
        {
            unsigned int w, h;
            getDisplayResolution(w, h);
            memcpy(displayData, data, w*h*3);
        }
        else
        {
            sonarData = (GLubyte*)data;
            newDataCallback(this);
            sonarData = NULL;
        }
    }
}

void FLS::InternalUpdate(Scalar dt)
{
    if(glFLS != nullptr)
        glFLS->Update();
}

std::vector<Renderable> FLS::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.type = RenderableType::SENSOR_LINES;    
        
        //Create sonar dummy
        GLfloat iconSize = range.y;
        int div = 12;
        GLfloat fovStep = glm::radians(fovH)/(GLfloat)div;
        //Min Arcs
        GLfloat cosVAngle = cosf(glm::radians(fovV)/2.f) * range.x;
        GLfloat sinVAngle = sinf(glm::radians(fovV)/2.f) * range.x;
        GLfloat hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Max Arcs
        cosVAngle = cosf(glm::radians(fovV)/2.f) * range.y;
        sinVAngle = sinf(glm::radians(fovV)/2.f) * range.y;
        hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = -fovStep*(div/2);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Ends
        hAngle = -fovStep*(div/2);
        GLfloat zs = cosf(hAngle) * cosVAngle;
        GLfloat xs = sinf(hAngle) * cosVAngle;
        item.points.push_back(glm::vec3(xs, sinVAngle, zs));
        item.points.push_back(glm::vec3(xs, -sinVAngle, zs));
        hAngle = fovStep*(div/2);
        GLfloat ze = cosf(hAngle) * cosVAngle;
        GLfloat xe = sinf(hAngle) * cosVAngle;
        item.points.push_back(glm::vec3(xe, sinVAngle, ze));
        item.points.push_back(glm::vec3(xe, -sinVAngle, ze));
        //Pyramid
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(xs, sinVAngle, zs));
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(xs, -sinVAngle, zs));
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(xe, sinVAngle, ze));
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(xe, -sinVAngle, ze));

        items.push_back(item);
    }
    return items;
}

}
