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
#include "graphics/OpenGLContent.h"

namespace sf
{

ColorCamera::ColorCamera(std::string uniqueName, uint32_t resX, uint32_t resY, Scalar horizFOVDeg, Scalar frequency, uint32_t spp, bool ao)
    : Camera(uniqueName, resX, resY, horizFOVDeg, frequency)
{
    spp = spp < 1 ? 1 : (spp > 16 ? 16 : spp);
    newDataCallback = NULL;
    imageData = new uint8_t[resX*resY*3]; //RGB color
    memset(imageData, 0, resX*resY*3);
    
    glCamera = new OpenGLRealCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)horizFOVDeg, 1000.f, spp, ao);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);
}

ColorCamera::~ColorCamera()
{
    if(imageData != NULL)
        delete imageData;
    
    glCamera = NULL;
}

uint8_t* ColorCamera::getDataPointer()
{
    return imageData;
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

void ColorCamera::NewDataReady()
{
    if(newDataCallback != NULL)
        newDataCallback(this);
}

void ColorCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
