//
//  OpenGLCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLCamera__
#define __Stonefish_OpenGLCamera__

#include "OpenGLView.h"
#include "Entity.h"

class OpenGLCamera : public OpenGLView
{
public:
    OpenGLCamera(const btVector3& eyePosition, const btVector3& targetPosition, const btVector3& cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLuint ssaoSize, GLfloat fov);
    ~OpenGLCamera();
    
    btTransform GetViewTransform();
    btVector3 GetEyePosition();
    btVector3 GetLookingDirection();
    btVector3 GetUpDirection();
    ViewType getType();
    
    void SetupCamera();
    void RenderDummy();
    void GlueToEntity(Entity* ent);
    void MoveCamera(const btVector3& move);
    void MoveCamera(btScalar step);
    void RotateCamera(btScalar panStep, btScalar tiltStep);
    
    void setPanAngle(GLfloat newPanAngle);
    GLfloat getPanAngle();
    void setTiltAngle(GLfloat newTiltAngle);
    GLfloat getTiltAngle();
    
private:
    btTransform cameraTransform;
    btTransform cameraRender;
    btVector3 eye;
    btVector3 dir;
    btVector3 up;
    
    Entity* holdingEntity;
    GLfloat pan;
    GLfloat tilt;
    btVector3 lookingDir;
};



#endif