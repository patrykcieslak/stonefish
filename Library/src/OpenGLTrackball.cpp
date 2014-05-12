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
    this->up = up;
    rotation = calculateRot(this->up, btVector3(0.,0.,1.));
    
    radius = UnitSystem::SetLength(orbitRadius);
    center = UnitSystem::SetPosition(centerPosition);
    fovx = fov/180.f*M_PI;
    dragging = false;
    holdingEntity = NULL;
    
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = (fovx/M_PI*180.f)/aspect;
    projection = glm::perspective(fovy, aspect, near, far);
    
    UpdateTrackballTransform();
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
    return center - radius * GetLookingDirection();
}

btVector3 OpenGLTrackball::GetLookingDirection()
{
    btVector3 dir(0,-1.f,0.f);
    dir = dir.rotate(rotation.getAxis(), -rotation.getAngle());
    dir.normalize();
    return dir;
}

btVector3 OpenGLTrackball::GetUpDirection()
{
    btVector3 localUp(0.,0.,1.f);
    localUp = localUp.rotate(rotation.getAxis(), -rotation.getAngle());
    localUp.normalize();
    return localUp;
}

void OpenGLTrackball::UpdateTrackballTransform()
{
    btVector3 upDir = GetUpDirection();
    btVector3 eyePos = GetEyePosition();
    
#ifdef BT_USE_DOUBLE_PRECISION
    glm::dvec3 eyeV(center.x(), center.y(), center.z());
    glm::dvec3 dirV(lookingDir.x(), lookingDir.y(), lookingDir.z());
    glm::dvec3 upV(upDir.x(), upDir.y(), upDir.z());
    glm::dmat4x4 cameraM = glm::lookAt(eyeV, eyeV+dirV, upV);
#else
    glm::vec3 centerV(center.x(), center.y(), center.z());
    glm::vec3 eyeV(eyePos.x(), eyePos.y(), eyePos.z());
    glm::vec3 upV(upDir.x(), upDir.y(), upDir.z());
    glm::mat4x4 cameraM = glm::lookAt(eyeV, centerV, upV);
#endif
    trackballTransform.setFromOpenGLMatrix(glm::value_ptr(cameraM));
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
        btQuaternion rotation_new = calculateRot(btVector3(-x_start, z_start, y_start).normalized(), btVector3(-x, z, y).normalized());
        rotation = rotation_new * rotation_start;
        UpdateTrackballTransform();
    }
}

void OpenGLTrackball::MouseScroll(GLfloat s)
{
    radius += s;
    if(radius < 0.5) radius = 0.5;
    UpdateTrackballTransform();
}

btTransform OpenGLTrackball::GetViewTransform()
{
    if(holdingEntity != NULL)
    {
        btTransform solidTrans = holdingEntity->getTransform();
        center = solidTrans.getOrigin();
        UpdateTrackballTransform();
        return trackballTransform;
        
        /*btTransform trans =  trackballTransform * solidTrans.inverse();
        btVector3 translate = solidTrans.getBasis() * GetEyePosition();
        trans.getOrigin() = trans.getOrigin() - translate;
        return trans;*/
    }
    else
    {
        return trackballTransform;
    }
}

void OpenGLTrackball::Rotate(const btQuaternion& rot)
{
    rotation = calculateRot(this->up, btVector3(0.,0.,1.f)) * rot;
    UpdateTrackballTransform();
}

void OpenGLTrackball::GlueToEntity(SolidEntity* solid)
{
    holdingEntity = solid;
}