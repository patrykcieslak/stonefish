//
//  OpenGLDepthCamera.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLDepthCamera__
#define __Stonefish_OpenGLDepthCamera__

#include "OpenGLView.h"
#include "SolidEntity.h"

class OpenGLDepthCamera : public OpenGLView
{
public:
    OpenGLDepthCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLfloat fovH, GLfloat minDepth, GLfloat maxDepth);
    ~OpenGLDepthCamera();
    
    void DrawLDR(GLuint destinationFBO);

protected:
    GLuint linearDepthFBO;
	GLuint linearDepthTex;
};

#endif