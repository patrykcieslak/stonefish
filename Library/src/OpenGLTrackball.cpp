//
//  OpenGLTrackball.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLTrackball.h"
#include "SimulationApp.h"
#include "MathsUtil.hpp"

OpenGLTrackball::OpenGLTrackball(const btVector3& centerPosition, btScalar orbitRadius, const btVector3& up, GLint x, GLint y, GLint width, GLint height, GLfloat fov, GLfloat horizon, GLuint spp, bool sao) : OpenGLView(x, y, width, height, horizon, spp, sao)
{
	this->up = glm::normalize(glm::vec3((GLfloat)up.getX(), (GLfloat)up.getY(), (GLfloat)up.getZ()));
    rotation = glm::rotation(this->up, glm::vec3(0,0,1.f));
	
    btVector3 _center = UnitSystem::SetPosition(centerPosition);
	center = glm::vec3((GLfloat)_center.getX(), (GLfloat)_center.getY(), (GLfloat)_center.getZ());
	radius = (GLfloat)UnitSystem::SetLength(orbitRadius);
    fovx = fov/180.f*M_PI;
    
	GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = fovx/aspect;
    projection = glm::perspective(fovy, aspect, near, far);
    
	dragging = false;
    transMode = false;
    holdingEntity = NULL;
    
    UpdateTrackballTransform();
}

OpenGLTrackball::~OpenGLTrackball()
{
}

ViewType OpenGLTrackball::getType()
{
    return TRACKBALL;
}

bool OpenGLTrackball::needsUpdate()
{
    return true;
}

glm::vec3 OpenGLTrackball::GetEyePosition() const
{
    return center - radius * GetLookingDirection();
}

glm::vec3 OpenGLTrackball::GetLookingDirection() const
{
    return glm::normalize(glm::vec3(glm::rotate(glm::inverse(rotation), glm::vec4(0,-1.f,0,1.f))));
}

glm::vec3 OpenGLTrackball::GetUpDirection() const
{
	glm::vec3 localUp = glm::vec3(0,0,1.f);	
    return glm::normalize(glm::vec3(glm::rotate(glm::inverse(rotation), glm::vec4(localUp, 1.f))));
}

void OpenGLTrackball::UpdateTrackballTransform()
{
	if(holdingEntity != NULL)
    {
		glm::mat4 solidTrans = glMatrixFromBtTransform(holdingEntity->getTransform());
        center = glm::vec3(solidTrans[3]);
	}
	
	trackballTransform = glm::lookAt(GetEyePosition(), center, GetUpDirection());
}

GLfloat OpenGLTrackball::calculateZ(GLfloat x, GLfloat y)
{
    if(x*x+y*y <= 0.5f)
        return sqrtf(1.f-(x*x+y*y));
    else
        return 0.5f/sqrtf(x*x+y*y);
}

void OpenGLTrackball::MouseDown(GLfloat x, GLfloat y, bool translate)
{
    x_start = x;
    y_start = y;
    
    if(translate)
    {
        transMode = true;
        translation_start = center;
    }
    else
    {
        transMode = false;
        z_start = calculateZ(x_start, y_start);
        rotation_start = rotation;
    }
    
    dragging = true;
}

void OpenGLTrackball::MouseUp()
{
    dragging = false;
}

void OpenGLTrackball::MouseMove(GLfloat x, GLfloat y)
{
    if(dragging)
    {
        if(transMode)
        {
            glm::vec3 right = glm::normalize(glm::cross(GetLookingDirection(), GetUpDirection()));
            center = translation_start + GetUpDirection() * (y-y_start) * -0.5f + right * (x-x_start) * -0.5f; 
        }
        else //rotate
        {
            GLfloat z = calculateZ(x, y);
            glm::quat rotation_new = glm::rotation(glm::normalize(glm::vec3(-x_start, z_start, y_start)), glm::normalize(glm::vec3(-x, z, y)));
            rotation = rotation_new * rotation_start;
        }
        
        UpdateTrackballTransform();
    }
}

void OpenGLTrackball::MouseScroll(GLfloat s)
{
    btScalar factor = pow(radius/5.0, 2.0);
    factor = factor > 1.0 ? 1.0 : factor;
    
    radius += s * factor;
    if(radius < 0.1) radius = 0.1;
    UpdateTrackballTransform();
}

glm::mat4 OpenGLTrackball::GetViewTransform() const
{
	return trackballTransform;
}

void OpenGLTrackball::Rotate(const btQuaternion& rot)
{
	glm::quat _rot((GLfloat)rot[0], (GLfloat)rot[1], (GLfloat)rot[2], (GLfloat)rot[3]);
	rotation = glm::rotation(this->up,  glm::vec3(0,0,1.f)) * _rot;
    UpdateTrackballTransform();
}

void OpenGLTrackball::MoveCenter(const glm::vec3& step)
{
    center += step;
    UpdateTrackballTransform();
}

void OpenGLTrackball::GlueToEntity(SolidEntity* solid)
{
    holdingEntity = solid;
}