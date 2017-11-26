//
//  OpenGLCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLCamera__
#define __Stonefish_OpenGLCamera__

#include "OpenGLView.h"

class OpenGLCamera : public OpenGLView
{
public:
    OpenGLCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLfloat fovH, GLfloat horizon, GLuint spp = 1, bool ao = false);
   
    glm::mat4 GetViewTransform() const;
    glm::vec3 GetEyePosition() const;
    glm::vec3 GetLookingDirection() const;
    glm::vec3 GetUpDirection() const;
    ViewType getType();
    
    void SetupCamera();
    void SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up);
    void RenderDummy();
    void RotateCamera(btScalar panStep, btScalar tiltStep);
    void Update();
    
	void setRendering(bool render); 
    void setPanAngle(GLfloat newPanAngle);
    GLfloat getPanAngle();
    void setTiltAngle(GLfloat newTiltAngle);
    GLfloat getTiltAngle();
    bool needsUpdate();
    
private:
    glm::mat4 cameraTransform;
    glm::mat4 cameraRender;
    glm::vec3 eye;
    glm::vec3 dir;
    glm::vec3 up;
	glm::vec3 tempEye;
	glm::vec3 tempDir;
	glm::vec3 tempUp;
    GLfloat pan;
    GLfloat tilt;
    glm::vec3 lookingDir;
    glm::vec3 currentUp;
    bool _needsUpdate;
};



#endif