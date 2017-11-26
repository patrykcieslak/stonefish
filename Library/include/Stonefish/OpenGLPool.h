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
    virtual ~OpenGLPool();
    
    void Init();
    void DrawSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport);
	void DrawBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport);
	void DrawBackground(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection);
	void DrawVolume(GLuint sceneTexture, GLuint linearDepthTex);
    
    void setLightAbsorptionCoeff(glm::vec3 a);
	void setTurbidity(GLfloat tur);
    glm::vec3 getLightAbsorptionCoeff();
	GLfloat getTurbidity();
    
private:    
    GLuint vao;
    GLuint vbo;
	std::vector<GLSLShader*> poolShaders;
    //GLSLShader* poolShaders[4]; //Surface, backsurface, background, volume
	GLfloat t;
    glm::vec3 lightAbsorption;
	GLfloat turbidity;
};

#endif