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
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/SSS.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLSSS.h"

#define SOUND_VELOCITY_WATER Scalar(1531) //Sea water

namespace sf
{

SSS::SSS(std::string uniqueName, unsigned int numOfBins, unsigned int numOfLines, Scalar verticalBeamWidthDeg,
         Scalar horizontalBeamWidthDeg, Scalar verticalTiltDeg, Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency)
    : Camera(uniqueName, (numOfBins%2==0 ? numOfBins : numOfBins+1), numOfLines, verticalBeamWidthDeg, frequency)
{
    range.x = 0.f;
    range.y = 0.f;
    setRangeMax(maxRange);
    setRangeMin(minRange);
    gain = Scalar(1);

    if(frequency < Scalar(0))
    {
        Scalar pulseTime = (Scalar(2)*range.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
        setUpdateFrequency(Scalar(1)/pulseTime);
    }
    fovV = horizontalBeamWidthDeg <= Scalar(0) ? Scalar(1) : (horizontalBeamWidthDeg > Scalar(90) ? Scalar(90) : horizontalBeamWidthDeg);
    tilt = verticalTiltDeg < Scalar(0) ? Scalar(0) : (verticalTiltDeg > Scalar(90) ? Scalar(90) : verticalTiltDeg);
    cMap = cm;
    sonarData = NULL;
    displayData = NULL;
    newDataCallback = NULL;
}

SSS::~SSS()
{
    if(displayData != NULL) delete [] displayData;
    glSSS = NULL;
}

void SSS::setRangeMin(Scalar r)
{
    range.x = r < Scalar(0.02) ? 0.02f : (r < Scalar(range.y) ? (GLfloat)r : range.x);
}

void SSS::setRangeMax(Scalar r)
{
    range.y = r > Scalar(range.x) ? (GLfloat)r : range.x;
    Scalar pulseTime = (Scalar(2)*range.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
    setUpdateFrequency(Scalar(1)/pulseTime);
}

void SSS::setGain(Scalar g)
{
    gain = g > Scalar(0) ? g : Scalar(1);
}

void* SSS::getImageDataPointer(unsigned int index)
{
    return sonarData;
}

void SSS::getDisplayResolution(unsigned int& x, unsigned int& y)
{
    GLint* viewport = glSSS->GetViewport();
    x = viewport[2];
    y = viewport[3];
    delete [] viewport;
}

GLubyte* SSS::getDisplayDataPointer()
{
    return displayData;
}

Scalar SSS::getRangeMin()
{
    return Scalar(range.x);
}

Scalar SSS::getRangeMax()
{
    return Scalar(range.y);
}

Scalar SSS::getGain()
{
    return gain;
}
   
VisionSensorType SSS::getVisionSensorType()
{
    return VisionSensorType::SENSOR_SSS;
}

void SSS::InitGraphics()
{
    glSSS = new OpenGLSSS(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, 
                          (GLfloat)fovH, (GLfloat)fovV, (GLint)resX, (GLint)resY, (GLfloat)tilt, range, false);
    glSSS->setSonar(this);
    glSSS->setColorMap(cMap);
    UpdateTransform();
    glSSS->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glSSS);

    unsigned int w, h;
    getDisplayResolution(w, h);
    displayData = new GLubyte[w*h*3];
}

void SSS::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glSSS->SetupSonar(eye_, dir_, up_);
}

void SSS::InstallNewDataHandler(std::function<void(SSS*)> callback)
{
    newDataCallback = callback;
}

void SSS::NewDataReady(void* data, unsigned int index)
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

void SSS::InternalUpdate(Scalar dt)
{
    glSSS->Update();
}

std::vector<Renderable> SSS::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;    
    
    //Create single transducer dummy
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

    //Add two transducer dummies
    GLfloat offsetAngle = M_PI_2 - glm::radians(tilt);
    glm::mat4 views[2];
    views[0] = glm::rotate(-offsetAngle, glm::vec3(0.f,1.f,0.f));
    views[1] = glm::rotate(offsetAngle, glm::vec3(0.f,1.f,0.f));
    item.model = glMatrixFromTransform(getSensorFrame()) * views[0];
    items.push_back(item);
    item.model = glMatrixFromTransform(getSensorFrame()) * views[1];
    items.push_back(item);
    
    return items;
}

}
