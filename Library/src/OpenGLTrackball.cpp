//
//  OpenGLTrackball.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLTrackball.h"

GLfloat x_start, y_start, z_start;
btQuaternion rotation_start;
bool dragging;

GLfloat calculateZ(GLfloat x, GLfloat y)
{
    if(x*x+y*y <= 0.5f)
        return sqrtf(1.f-(x*x+y*y));
    else
        return 0.5f/sqrtf(x*x+y*y);
}

btQuaternion calculateRot(btVector3 v_start, btVector3 v)
{
    btVector3 axis = v_start.cross(v);
    
    if(axis.length() > 0)
    {
        btScalar angle = acos(v_start.dot(v));
        return btQuaternion(axis, angle);
    }
    else
        return btQuaternion::getIdentity();
}

OpenGLTrackball::OpenGLTrackball(const btVector3& centerPosition, btScalar orbitRadius, const btVector3& up, GLint x, GLint y, GLint width, GLint height, GLuint ssaoSize, GLfloat fov) : OpenGLView(x, y, width, height, ssaoSize)
{
    rotation = btQuaternion(0, 0, 0);
    rotation = calculateRot(btVector3(0,-1,0), up) * rotation;
    
    radius = UnitSystem::SetLength(orbitRadius);
    center = UnitSystem::SetPosition(centerPosition);
    fovx = UnitSystem::SetAngle(fov);
    dragging = false;
    
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = (fovx/M_PI*180.f)/aspect;
    projection = glm::perspective(fovy, aspect, near, far);
}

OpenGLTrackball::~OpenGLTrackball()
{
}

ViewType OpenGLTrackball::getType()
{
    return TRACKBALL;
}

btVector3 OpenGLTrackball::GetEyePosition()
{
    btVector3 dir = btVector3(0,0,-1);
    dir = dir.rotate(rotation.getAxis(), -rotation.getAngle());
    dir.normalize();
    return center-radius*dir;
}

btVector3 OpenGLTrackball::GetLookingDirection()
{
    return (center-GetEyePosition()).normalized();
}

void OpenGLTrackball::MouseDown(GLfloat x, GLfloat y)
{
    x_start = x;
    y_start = y;
    z_start = calculateZ(x_start, y_start);
    rotation_start = rotation;
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
        GLfloat z = calculateZ(x, y);
        btQuaternion rotation_new = calculateRot(btVector3(x_start, y_start, z_start).normalized(), btVector3(x, y, z).normalized());
        rotation = rotation_new * rotation_start;
    }
}

void OpenGLTrackball::MouseScroll(GLfloat s)
{
    radius += s;
}

btTransform OpenGLTrackball::GetViewTransform()
{
    btTransform transR = btTransform::getIdentity();
    transR.setOrigin(btVector3(0,0,-radius));
    
    btTransform trans = btTransform(rotation, -center);
    
    return transR * trans;
}

void OpenGLTrackball::Rotate(const btQuaternion& rot)
{
    rotation = rot * rotation;
}