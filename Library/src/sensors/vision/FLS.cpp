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
    Scalar verticalFOVDeg, Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency, unsigned int beamHPix, unsigned int beamVPix)
    : Camera(uniqueName, numOfBeams, numOfBins, horizontalFOVDeg, frequency)
{
    range.x = minRange < Scalar(0.01) ? 0.01f : (GLfloat)minRange;
    range.y = maxRange > Scalar(0.01) ? (GLfloat)maxRange : 0.1f;
    fovV = verticalFOVDeg <= Scalar(0) ? Scalar(20) : (verticalFOVDeg > Scalar(180) ? Scalar(180) : verticalFOVDeg);
    cMap = cm;
    beamRes.x = beamHPix == 0 ? 5 : (GLint)beamHPix;
    beamRes.y = beamVPix == 0 ? (GLint)ceil(fovV*beamRes.x*range.y) : (GLint)beamVPix;
    newDataCallback = NULL;
    sonarData = new GLfloat[resX*resY]; // Buffer for storing range data
    displayData = NULL;
    memset(sonarData, 0, resX*resY*sizeof(GLfloat));
}

FLS::~FLS()
{
    if(sonarData != NULL)
        delete [] sonarData;
    if(displayData != NULL)
        delete [] displayData;
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

GLuint* FLS::getDisplayDataPointer()
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
    glFLS = new OpenGLFLS(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, (GLfloat)fovH, (GLfloat)fovV, 
                          (GLint)resX, beamRes.x, beamRes.y, range.x, range.y, (GLint)resY);
    glFLS->setSonar(this);
    glFLS->setColorMap(cMap);
    UpdateTransform();
    glFLS->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glFLS);
    
    GLint* viewport = glFLS->GetViewport();
    displayData = new GLuint[viewport[2] * viewport[3] * 3];
    delete [] viewport;
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

void FLS::NewDataReady(unsigned int index)
{
    if(newDataCallback != NULL)
        newDataCallback(this);
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
    
    //Create camera dummy
    GLfloat iconSize = 0.5f;
    GLfloat x = iconSize*tanf(fovH/360.f*M_PI);
    GLfloat y = iconSize*tanf(fovV/360.f*M_PI);
    
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(x, -y, iconSize));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(x,  y, iconSize));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-x, -y, iconSize));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-x,  y, iconSize));
    
    item.points.push_back(glm::vec3(x, -y, iconSize));
    item.points.push_back(glm::vec3(x, y, iconSize));
    item.points.push_back(glm::vec3(x, y, iconSize));
    item.points.push_back(glm::vec3(-x, y, iconSize));
    item.points.push_back(glm::vec3(-x, y, iconSize));
    item.points.push_back(glm::vec3(-x, -y, iconSize));
    item.points.push_back(glm::vec3(-x, -y, iconSize));
    item.points.push_back(glm::vec3(x, -y, iconSize));
    
    item.points.push_back(glm::vec3(-0.5f*x, -y, iconSize));
    item.points.push_back(glm::vec3(0.f, -1.5f*y, iconSize));
    item.points.push_back(glm::vec3(0.f, -1.5f*y, iconSize));
    item.points.push_back(glm::vec3(0.5f*x, -y, iconSize));
    
    items.push_back(item);
    return items;
}

}
