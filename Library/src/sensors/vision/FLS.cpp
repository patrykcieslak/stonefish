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

#define SOUND_VELOCITY_WATER Scalar(1531) //Sea water

namespace sf
{

FLS::FLS(std::string uniqueName, unsigned int numOfBeams, unsigned int numOfBins, Scalar horizontalFOVDeg, 
    Scalar verticalFOVDeg, Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency)
    : Camera(uniqueName, numOfBeams, numOfBins, horizontalFOVDeg, frequency)
{
    range.x = minRange < Scalar(0.01) ? 0.01f : (GLfloat)minRange;
    range.y = maxRange > Scalar(0.01) ? (GLfloat)maxRange : 0.1f;
    if(frequency < Scalar(0))
    {
        Scalar pulseTime = (Scalar(2)*range.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
        setUpdateFrequency(Scalar(1)/pulseTime);
    }
    fovV = verticalFOVDeg <= Scalar(0) ? Scalar(20) : (verticalFOVDeg > Scalar(179) ? Scalar(179) : verticalFOVDeg);
    cMap = cm;
    sonarData = NULL;
    displayData = NULL;
    newDataCallback = NULL;
}

FLS::~FLS()
{
    if(displayData != NULL) delete [] displayData;
    glFLS = NULL;
}

void* FLS::getImageDataPointer(unsigned int index)
{
    return sonarData;
}

void FLS::getDisplayResolution(unsigned int& x, unsigned int& y)
{
    GLint* viewport = glFLS->GetViewport();
    x = viewport[2];
    y = viewport[3];
    delete [] viewport;
}

GLubyte* FLS::getDisplayDataPointer()
{
    return displayData;
}

glm::vec2 FLS::getRangeLimits()
{
    return range;
}
    
VisionSensorType FLS::getVisionSensorType()
{
    return VisionSensorType::SENSOR_FLS;
}

void FLS::InitGraphics()
{
    glFLS = new OpenGLFLS(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, 
                           (GLfloat)fovH, (GLfloat)fovV, (GLint)resX, (GLint)resY, range, freq < Scalar(0));
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
            sonarData = (GLfloat*)data;
            newDataCallback(this);
            sonarData = NULL;
        }
    }
}

void FLS::InternalUpdate(Scalar dt)
{
    glFLS->Update();
}

std::vector<Renderable> FLS::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.model = glMatrixFromTransform(getSensorFrame());
    item.type = RenderableType::SENSOR_LINES;    
    
    //Create sonar dummy
    GLfloat iconSize = 0.5f;
    int div = 12;
    GLfloat fovStep = glm::radians(fovH)/(GLfloat)div;
    GLfloat cosVAngle = cosf(glm::radians(fovV)/2.f) * iconSize;
    GLfloat sinVAngle = sinf(glm::radians(fovV)/2.f) * iconSize;
    //Arcs
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
    return items;
}

}
