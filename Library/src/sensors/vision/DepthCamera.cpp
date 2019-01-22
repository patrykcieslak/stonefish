//
//  DepthCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 07/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/DepthCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLDepthCamera.h"

namespace sf
{

DepthCamera::DepthCamera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizontalFOVDeg, Scalar minDepth, Scalar maxDepth, Scalar frequency)
    : Camera(uniqueName, resolutionX, resolutionY, horizontalFOVDeg, frequency)
{
    newDataCallback = NULL;
    depthRange.x = minDepth < Scalar(0.01) ? 0.01f : (GLfloat)minDepth;
    depthRange.y = maxDepth > Scalar(0.01) ? (GLfloat)maxDepth : 1.f;
    imageData = new GLfloat[resX*resY]; //float depth
    memset(imageData, 0, resX*resY*sizeof(GLfloat));
}

DepthCamera::~DepthCamera()
{
    if(imageData != NULL)
        delete imageData;
    glCamera = NULL;
}

void* DepthCamera::getImageDataPointer(unsigned int index)
{
    return imageData;
}

glm::vec2 DepthCamera::getDepthRange()
{
    return depthRange;
}
    
void DepthCamera::InitGraphics()
{
    glCamera = new OpenGLDepthCamera(glm::vec3(0,0,0), glm::vec3(0,0,1.f), glm::vec3(0,-1.f,0), 0, 0, resX, resY, (GLfloat)fovH, depthRange.x, depthRange.y);
    glCamera->setCamera(this);
    UpdateTransform();
    glCamera->UpdateTransform();
    InternalUpdate(0);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddView(glCamera);
}

void DepthCamera::SetupCamera(const Vector3& eye, const Vector3& dir, const Vector3& up)
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

void DepthCamera::NewDataReady(unsigned int index)
{
    if(newDataCallback != NULL)
        newDataCallback(this);
}

void DepthCamera::InternalUpdate(Scalar dt)
{
    glCamera->Update();
}

}
