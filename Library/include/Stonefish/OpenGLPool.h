//
//  OpenGLPool.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPool__
#define __Stonefish_OpenGLPool__

#include "OpenGLContent.h"
#include "GLSLShader.h"

class OpenGLPool
{
public:
    OpenGLPool();
    ~OpenGLPool();
    
    void InitPool();
    void DrawPoolSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture);
	void DrawPoolBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection);
	void DrawPoolVolume(GLuint sceneTexture, GLuint linearDepthTex);
    
private:    
    GLuint vao;
    GLuint vbo;
    GLSLShader* poolShaders[3]; //Surface, backsurface, volume
};

#endif