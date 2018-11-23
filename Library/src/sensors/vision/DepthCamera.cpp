//
//  DepthCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 07/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/DepthCamera.h"

using namespace sf;

DepthCamera::DepthCamera(std::string uniqueName, uint32_t resX, uint32_t resY, btScalar horizFOVDeg, btScalar minDepth, btScalar maxDepth, btScalar frequency)
    : Camera(uniqueName, resX, resY, horizFOVDeg, frequency)
{
    newDataCallback = NULL;
    depthRange.x = minDepth;
    depthRange.y = maxDepth;
    imageData = new GLfloat[resX*resY]; //float depth
    memset(imageData, 0, resX*resY*sizeof(GLfloat));
    
    glCamera = new OpenGLDepthCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)horizFOVDeg, (GLfloat)minDepth, (GLfloat)maxDepth);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    OpenGLContent::getInstance()->AddView(glCamera);
}

DepthCamera::~DepthCamera()
{
    if(imageData != NULL)
        delete imageData;
    glCamera = NULL;
}

GLfloat* DepthCamera::getDataPointer()
{
    return imageData;
}

glm::vec2 DepthCamera::getDepthRange()
{
    return depthRange;
}

void DepthCamera::SetupCamera(const btVector3& eye, const btVector3& dir, const btVector3& up)
{
    glm::vec3 eye_ = glm::vec3((GLfloat)eye.x(), (GLfloat)eye.y(), (GLfloat)eye.z());
    glm::vec3 dir_ = glm::vec3((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
    glm::vec3 up_ = glm::vec3((GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z());
    glCamera->SetupCamera(eye_, dir_, up_);
}

void DepthCamera::InstallNewDataHandler(std::function<void(DepthCamera*)> callback)
{
    newDataCallback = callback;
}

void DepthCamera::NewDataReady()
{
    if(newDataCallback != NULL)
        newDataCallback(this);
}

void DepthCamera::InternalUpdate(btScalar dt)
{
    glCamera->Update();
}
