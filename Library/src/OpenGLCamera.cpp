//
//  OpenGLCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLCamera.h"
#include "MathsUtil.hpp"

OpenGLCamera::OpenGLCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint x, GLint y, GLint width, GLint height, GLfloat fovH, GLfloat horizon, GLuint spp, bool sao) : OpenGLView(x, y, width, height, horizon, spp, sao)
{
    _needsUpdate = false;
    
    //Setup view
    pan = 0.f;
    tilt = 0.f;
    SetupCamera(eyePosition, direction, cameraUp);

    //Setup projection
    fovx = fovH/180.f*M_PI;
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = fovx/aspect;
    projection = glm::perspective(fovy, aspect, near, far);
}

ViewType OpenGLCamera::getType()
{
    return CAMERA;
}

void OpenGLCamera::RotateCamera(btScalar panStep, btScalar tiltStep)
{
    pan += UnitSystem::SetAngle(panStep);
    tilt += UnitSystem::SetAngle(tiltStep);
    SetupCamera();
}

void OpenGLCamera::Update()
{
    _needsUpdate = true;
}

void OpenGLCamera::setRendering(bool render)
{
	if(!rendering && render)
	{
		dir = tempDir;
		eye = tempEye;
		up = tempUp;
		SetupCamera();
	}
	
	rendering = render;
}

bool OpenGLCamera::needsUpdate()
{
    bool nu = _needsUpdate;
    _needsUpdate = false;
    return nu;
}

void OpenGLCamera::setPanAngle(GLfloat newPanAngle)
{
    newPanAngle = UnitSystem::SetAngle(newPanAngle);
    pan = newPanAngle;
    SetupCamera();
}

void OpenGLCamera::setTiltAngle(GLfloat newTiltAngle)
{
    newTiltAngle = UnitSystem::SetAngle(newTiltAngle);
    tilt = newTiltAngle;
    SetupCamera();
}


GLfloat OpenGLCamera::getPanAngle()
{
    return UnitSystem::GetAngle(pan);
}

GLfloat OpenGLCamera::getTiltAngle()
{
    return UnitSystem::GetAngle(tilt);
}

glm::vec3 OpenGLCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLCamera::GetLookingDirection() const
{
    return lookingDir;
}

glm::vec3 OpenGLCamera::GetUpDirection() const
{
    return currentUp;
}

void OpenGLCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
	if(!rendering)
	{
		dir = _dir;
		eye = _eye;
		up = _up;
		SetupCamera();
	}
	
	tempDir = _dir;
	tempEye = _eye;
	tempUp = _up;
}

void OpenGLCamera::SetupCamera()
{
    lookingDir = dir;
    currentUp = up;
    
    //additional camera rotation
    glm::vec3 tiltAxis = glm::normalize(glm::cross(dir, up));
    glm::vec3 panAxis = glm::normalize(glm::cross(tiltAxis, dir));
    
    //rotate
	lookingDir = glm::rotate(lookingDir, tilt, tiltAxis);
	lookingDir = glm::rotate(lookingDir, pan, panAxis);
    lookingDir = glm::normalize(lookingDir);
    
    currentUp = glm::rotate(currentUp, tilt, tiltAxis);
    currentUp = glm::rotate(currentUp, pan, panAxis);
	currentUp = glm::normalize(currentUp);
    
	cameraTransform = glm::lookAt(eye, eye+lookingDir, currentUp);
	cameraRender = glm::inverse(cameraTransform);
}

glm::mat4 OpenGLCamera::GetViewTransform() const
{
    return cameraTransform;
}
