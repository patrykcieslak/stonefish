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

class OpenGLCamera : public OpenGLView
{
public:
    OpenGLCamera(const btVector3& eyePosition, const btVector3& targetPosition, const btVector3& cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLfloat fov, GLfloat horizon, bool sao = false);
    ~OpenGLCamera();
    
    glm::mat4 GetViewTransform();
    glm::vec3 GetEyePosition();
    glm::vec3 GetLookingDirection();
    glm::vec3 GetUpDirection();
    ViewType getType();
    
    void SetupCamera();
    void RenderDummy();
    void GlueToEntity(SolidEntity* ent);
    void MoveCamera(const btVector3& move);
    void MoveCamera(btScalar step);
    void RotateCamera(btScalar panStep, btScalar tiltStep);
    
    void setPanAngle(GLfloat newPanAngle);
    GLfloat getPanAngle();
    void setTiltAngle(GLfloat newTiltAngle);
    GLfloat getTiltAngle();
    
private:
    glm::mat4 cameraTransform;
    glm::mat4 cameraRender;
    glm::vec3 eye;
    glm::vec3 dir;
    glm::vec3 up;
    
    SolidEntity* holdingEntity;
    GLfloat pan;
    GLfloat tilt;
    glm::vec3 lookingDir;
};



#endif