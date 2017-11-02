//
//  Camera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Camera.h"

Camera::Camera(std::string uniqueName, const btVector3& eyePosition, const btVector3& targetPosition, const btVector3& cameraUp, 
               GLint originX, GLint originY, GLint width, GLint height, GLfloat fov, btScalar frequency, bool advancedRendering) : Sensor(uniqueName, frequency)
{
    glCamera = new OpenGLCamera(eyePosition, targetPosition, cameraUp, originX, originY, width, height, fov, 1000.f, advancedRendering);
    glCamera->Activate();
    OpenGLContent::getInstance()->AddView(glCamera);
}
    
Camera::~Camera()
{
    glCamera = NULL;
}

void Camera::InternalUpdate(btScalar dt)
{
}

std::vector<Renderable> Camera::Render()
{
    std::vector<Renderable> items(0);
    return items;
}

SensorType Camera::getType()
{
    return SensorType::SENSOR_CAMERA;
}