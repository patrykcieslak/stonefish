//
//  ColorCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/5/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include <sensors/ColorCamera.h>

ColorCamera::ColorCamera(std::string uniqueName, uint32_t resX, uint32_t resY, btScalar horizFOVDeg, const btTransform& geomToSensor, SolidEntity* attachment, btScalar frequency, uint32_t spp, bool ao) 
    : Camera(uniqueName, resX, resY, horizFOVDeg, geomToSensor, attachment, frequency)
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
    OpenGLContent::getInstance()->AddView(glCamera);
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

void ColorCamera::SetupCamera(const btVector3& eye, const btVector3& dir, const btVector3& up)
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

void ColorCamera::InternalUpdate(btScalar dt)
{
    glCamera->Update();
}