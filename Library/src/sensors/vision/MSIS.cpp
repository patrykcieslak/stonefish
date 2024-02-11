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
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/MSIS.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLMSIS.h"

namespace sf
{

MSIS::MSIS(std::string uniqueName, Scalar stepAngleDeg, unsigned int numOfBins, Scalar horizontalBeamWidthDeg, Scalar verticalBeamWidthDeg,
           Scalar minRotationDeg, Scalar maxRotationDeg, Scalar minRange, Scalar maxRange, ColorMap cm, Scalar frequency)
    : Camera(uniqueName, (unsigned int)ceil(Scalar(360)/stepAngleDeg), numOfBins, horizontalBeamWidthDeg, frequency)
{
    range.x = 0.f;
    range.y = 0.f;
    noise = glm::vec2(0.f);
    fullRotation = false;
    stepSize = btRadians(btScalar(360)/ceil(Scalar(360)/stepAngleDeg)); //Corrected step angle in radians
    setRotationLimits(minRotationDeg, maxRotationDeg);
    setRangeMax(maxRange);
    setRangeMin(minRange);
    setGain(1);
    fovV = verticalBeamWidthDeg <= Scalar(0) ? Scalar(20) : (verticalBeamWidthDeg > Scalar(179) ? Scalar(179) : verticalBeamWidthDeg);
    cMap = cm;
    sonarData = NULL;
    displayData = NULL;
    newDataCallback = NULL;
    glMSIS = nullptr;
}

MSIS::~MSIS()
{
    if(displayData != NULL) delete [] displayData;
    glMSIS = nullptr;
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
    roi.x = (GLint)round(btRadians(l1Deg)/stepSize);
    roi.y = (GLint)round(btRadians(l2Deg)/stepSize);
    fullRotation = (roi.x == -(int)resX/2) && (roi.y == (int)resX/2);
    if(roi.y == (int)resX/2)
        --roi.y;
    currentStep = roi.x;
    cw = true;
}

void MSIS::setRangeMin(Scalar r)
{
    range.x = r < Scalar(0.02) ? 0.02f : (r < Scalar(range.y) ? (GLfloat)r : range.x);
}

void MSIS::setRangeMax(Scalar r)
{
    range.y = r > Scalar(range.x) ? (GLfloat)r : range.x;
    Scalar pulseTime = (Scalar(2)*range.y/SOUND_VELOCITY_WATER) * Scalar(1.1);
    setUpdateFrequency(Scalar(1)/pulseTime);
}

void MSIS::setGain(Scalar g)
{
    gain = g > Scalar(0) ? g : Scalar(1);
}

void MSIS::setNoise(float multiplicativeStdDev, float additiveStdDev)
{
    if(multiplicativeStdDev >= 0.f)
        noise.x = multiplicativeStdDev;
    if(additiveStdDev >= 0.f)
        noise.y = additiveStdDev;
    if(glMSIS != nullptr)
        glMSIS->setNoise(noise);
}

void* MSIS::getImageDataPointer(unsigned int index)
{
    return sonarData;
}

void MSIS::getDisplayResolution(unsigned int& x, unsigned int& y)
{
    getResolution(x, y);
    x = y; //numOfBins x numOfBins
}

GLubyte* MSIS::getDisplayDataPointer()
{
    return displayData;
}

void MSIS::getRotationLimits(Scalar& l1Deg, Scalar& l2Deg) const
{
    l1Deg = btDegrees(Scalar(roi.x) * stepSize);
    l2Deg = btDegrees(Scalar(roi.y) * stepSize);
}

Scalar MSIS::getRotationStepAngle() const
{
    return btDegrees(stepSize);
}

int MSIS::getCurrentRotationStep() const
{
    return currentStep;
}

Scalar MSIS::getRangeMin() const
{
    return Scalar(range.x);
}

Scalar MSIS::getRangeMax() const
{
    return Scalar(range.y);
}

Scalar MSIS::getGain() const
{
    return gain;
}
    
VisionSensorType MSIS::getVisionSensorType()
{
    return VisionSensorType::MSIS;
}

void MSIS::InitGraphics()
{
    glMSIS = new OpenGLMSIS(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0),
                           (GLfloat)fovH, (GLfloat)fovV, (GLint)resX, (GLint)resY, range);
    glMSIS->setNoise(noise);
    glMSIS->setSonar(this);
    glMSIS->setColorMap(cMap);
    UpdateTransform();
    glMSIS->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glMSIS);

    unsigned int w, h;
    getDisplayResolution(w, h);
    displayData = new GLubyte[w*h*3];
}

void MSIS::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glMSIS->SetupSonar(eye_, dir_, up_);
}

void MSIS::InstallNewDataHandler(std::function<void(MSIS*)> callback)
{
    newDataCallback = callback;
}

void MSIS::NewDataReady(void* data, unsigned int index)
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

    if(index == 1)
    {
        currentStep += cw ? 1 : -1;
        
        if(fullRotation)
        {
            if(cw && currentStep > roi.y)
                currentStep = roi.x;
            else if(!cw && currentStep < roi.x)
                currentStep = roi.y;
        }
        else
        {
            if(cw && currentStep == roi.y)
                cw = false;
            else if(!cw && currentStep == roi.x)
                cw = true;
        }
    }
}

void MSIS::InternalUpdate(Scalar dt)
{
    if(glMSIS != nullptr)
        glMSIS->Update();
}

std::vector<Renderable> MSIS::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.type = RenderableType::SENSOR_LINES;    
        
        //Create sonar dummy
        int div = 24;
        Scalar l1Deg, l2Deg;
        getRotationLimits(l1Deg, l2Deg);
        GLfloat fovStep = fullRotation ? 2.f*M_PI/(GLfloat)div : glm::radians(l2Deg-l1Deg)/(GLfloat)div;
        //Arcs min
        GLfloat cosVAngle = cosf(glm::radians(fovV)/2.f) * range.x;
        GLfloat sinVAngle = sinf(glm::radians(fovV)/2.f) * range.x;
        GLfloat hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Arcs max
        cosVAngle = cosf(glm::radians(fovV)/2.f) * range.y;
        sinVAngle = sinf(glm::radians(fovV)/2.f) * range.y;
        hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, sinVAngle, z));
            hAngle += fovStep;
        }
        hAngle = glm::radians(l1Deg);
        for(int i=0; i<=div; ++i)
        {
            GLfloat z = cosf(hAngle) * cosVAngle;
            GLfloat x = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(x, -sinVAngle, z));
            if(i > 0 && i < div)
                item.points.push_back(glm::vec3(x, -sinVAngle, z));
            hAngle += fovStep;
        }
        //Current beam position
        hAngle = currentStep * stepSize;
        GLfloat zc = cosf(hAngle) * cosVAngle;
        GLfloat xc = sinf(hAngle) * cosVAngle;
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(xc, sinVAngle, zc));
        item.points.push_back(glm::vec3(xc, sinVAngle, zc));
        item.points.push_back(glm::vec3(xc, -sinVAngle, zc));
        item.points.push_back(glm::vec3(xc, -sinVAngle, zc));
        item.points.push_back(glm::vec3(0,0,0));
        
        if(!fullRotation)
        {
            //Ends
            hAngle = glm::radians(l1Deg);
            GLfloat zs = cosf(hAngle) * cosVAngle;
            GLfloat xs = sinf(hAngle) * cosVAngle;
            item.points.push_back(glm::vec3(xs, sinVAngle, zs));
            item.points.push_back(glm::vec3(xs, -sinVAngle, zs));
            hAngle = glm::radians(l2Deg);
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
        }

        items.push_back(item);
    }
    return items;
}

}
