//
//  ColorCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/5/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/ColorCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLRealCamera.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

ColorCamera::ColorCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, unsigned char spp, Scalar frequency)
    : Camera(uniqueName, resolutionX, resolutionY, horizFOVDeg, frequency)
{
    samples = spp < 1 ? 1 : (spp > 16 ? 16 : spp);
    newDataCallback = NULL;
    imageData = new uint8_t[resX*resY*3]; //RGB color
    memset(imageData, 0, resX*resY*3);
}

ColorCamera::~ColorCamera()
{
    if(imageData != NULL)
        delete imageData;
    
    glCamera = NULL;
}
    
void ColorCamera::setExposureCompensation(Scalar comp)
{
    if(glCamera != NULL)
        glCamera->setExposureCompensation((GLfloat)comp);
}
    
Scalar ColorCamera::getExposureCompensation()
{
    if(glCamera != NULL)
        return (Scalar)glCamera->getExposureCompensation();
    else
        return Scalar(0);
}

void* ColorCamera::getImageDataPointer(unsigned int index)
{
    return imageData;
}
    
void ColorCamera::InitGraphics()
{
    glCamera = new OpenGLRealCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, 1000.f, samples, true);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);
}

void ColorCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void ColorCamera::InstallNewDataHandler(std::function<void(ColorCamera*)> callback)
{
    newDataCallback = callback;
}

void ColorCamera::NewDataReady(unsigned int index)
{
    if(newDataCallback != NULL)
        newDataCallback(this);
}

void ColorCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
