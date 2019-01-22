//
//  Camera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/Camera.h"

#include "entities/SolidEntity.h"

namespace sf
{

Camera::Camera(std::string uniqueName, unsigned int resolutionX, unsigned int resolutionY, Scalar horizFOVDeg, Scalar frequency) : VisionSensor(uniqueName, frequency)
{
    fovH = horizFOVDeg <= Scalar(0) ? Scalar(90) : (horizFOVDeg > Scalar(360) ? Scalar(360) : horizFOVDeg);
    resX = resolutionX > 0 ? resolutionX : 1;
    resY = resolutionY > 0 ? resolutionY : 1;
    display = false;
}
    
Camera::~Camera()
{
}

Scalar Camera::getHorizontalFOV()
{
    return fovH;
}

void Camera::getResolution(unsigned int& x, unsigned int& y)
{
    x = resX;
    y = resY;
}

void Camera::setDisplayOnScreen(bool onScreen)
{
    display = onScreen;
}

bool Camera::getDisplayOnScreen()
{
    return display;
}

void Camera::UpdateTransform()
{
    Transform cameraTransform = getSensorFrame();
    Vector3 eyePosition = cameraTransform.getOrigin(); //O
    Vector3 direction = cameraTransform.getBasis().getColumn(2); //Z
    Vector3 cameraUp = -cameraTransform.getBasis().getColumn(1); //-Y
    
    Matrix3 rotation;
    rotation.setEulerYPR(0,M_PI,0);
    Vector3 rotEyePosition = rotation * eyePosition;
    Vector3 rotDirection = rotation * direction;
    Vector3 rotCameraUp = rotation * cameraUp;
    SetupCamera(rotEyePosition, rotDirection, rotCameraUp);
}

std::vector<Renderable> Camera::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.model = glMatrixFromTransform(getSensorFrame());
    item.type = RenderableType::SENSOR_LINES;
    
    //Create camera dummy
    GLfloat iconSize = 0.5f;
    GLfloat x = iconSize*tanf(fovH/360.f*M_PI);
    GLfloat aspect = (GLfloat)resX/(GLfloat)resY;
    GLfloat y = x/aspect;
	
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
