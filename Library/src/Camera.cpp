//
//  Camera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Camera.h"
#include "SimulationApp.h"
#include "MathsUtil.hpp"

Camera::Camera(std::string uniqueName, unsigned int resX, unsigned int resY, btScalar horizFOV, const btTransform& geomToSensor, SolidEntity* attachment, btScalar frequency, unsigned int spp, bool ao) : Sensor(uniqueName, frequency)
{
    g2s = UnitSystem::SetTransform(geomToSensor);
    attach = attachment;
    fovH = horizFOV;
    resx = resX;
    resy = resY;
    renderSpp = spp < 1 ? 1 : (spp > 16 ? 16 : spp);
    renderAO = ao;
    
    glCamera = new OpenGLCamera(glm::vec3(0,0,0), glm::vec3(1.f,0,0), glm::vec3(0,0,1.f), 0, 0, resx, resy, (GLfloat)fovH, 1000.f, renderSpp, renderAO);
    UpdateTransform();
    InternalUpdate(0);
    OpenGLContent::getInstance()->AddView(glCamera);
}
    
Camera::~Camera()
{
    glCamera = NULL;
}

void Camera::UpdateTransform()
{
    btTransform cameraTransform;
    
    if(attach != NULL)
        cameraTransform = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    else
        cameraTransform = g2s;
    
    btVector3 eyePosition = cameraTransform.getOrigin();
    btVector3 direction = cameraTransform.getBasis().getColumn(0);
    btVector3 cameraUp = cameraTransform.getBasis().getColumn(2);
    
    bool zUp = SimulationApp::getApp()->getSimulationManager()->isZAxisUp();
    
    if(zUp)
    {
        glm::vec3 eye = glm::vec3((GLfloat)eyePosition.x(), (GLfloat)eyePosition.y(), (GLfloat)eyePosition.z());
        glm::vec3 dir = glm::vec3((GLfloat)direction.x(), (GLfloat)direction.y(), (GLfloat)direction.z());
        glm::vec3 up = glm::vec3((GLfloat)cameraUp.x(), (GLfloat)cameraUp.y(), (GLfloat)cameraUp.z());
        glCamera->SetupCamera(eye, dir, up);
    }
    else
    {
        btMatrix3x3 rotation;
        rotation.setEulerYPR(0,M_PI,0);
        btVector3 rotEyePosition = rotation * eyePosition;
        btVector3 rotDirection = rotation * direction;
        btVector3 rotCameraUp = rotation * cameraUp;
        glm::vec3 eye = glm::vec3((GLfloat)rotEyePosition.x(), (GLfloat)rotEyePosition.y(), (GLfloat)rotEyePosition.z());
        glm::vec3 dir = glm::vec3((GLfloat)rotDirection.x(), (GLfloat)rotDirection.y(), (GLfloat)rotDirection.z());
        glm::vec3 up = glm::vec3((GLfloat)rotCameraUp.x(), (GLfloat)rotCameraUp.y(), (GLfloat)rotCameraUp.z());
        glCamera->SetupCamera(eye, dir, up);
    }
}

void Camera::InternalUpdate(btScalar dt)
{
    glCamera->Update();
}

std::vector<Renderable> Camera::Render()
{
    std::vector<Renderable> items(0);
    
    btTransform cameraTransform;
    
    if(attach != NULL)
        cameraTransform = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    else
        cameraTransform = g2s;
    
    Renderable item;
    item.model = glMatrixFromBtTransform(cameraTransform);
    item.type = RenderableType::SENSOR_LINES;
    
    //Create camera dummy
    GLfloat iconSize = 1.f;
    GLfloat x = iconSize*tanf(fovH/360.f*M_PI);
    GLfloat aspect = (GLfloat)resx/(GLfloat)resy;
    GLfloat y = x/aspect;
	
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(iconSize, x, -y));
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(iconSize, x,  y));
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(iconSize,-x, -y));
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(iconSize,-x,  y));
	
	item.points.push_back(glm::vec3(iconSize, x, -y));
	item.points.push_back(glm::vec3(iconSize, x, y));
	item.points.push_back(glm::vec3(iconSize, x, y));
	item.points.push_back(glm::vec3(iconSize, -x, y));
	item.points.push_back(glm::vec3(iconSize, -x, y));
	item.points.push_back(glm::vec3(iconSize, -x, -y));
	item.points.push_back(glm::vec3(iconSize, -x, -y));
	item.points.push_back(glm::vec3(iconSize, x, -y));
	
    items.push_back(item);
    return items;
}

SensorType Camera::getType()
{
    return SensorType::SENSOR_CAMERA;
}