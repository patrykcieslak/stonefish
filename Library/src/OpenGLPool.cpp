//
//  OpenGLPool.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPool.h"
#include "Console.h"

OpenGLPool::OpenGLPool()
{
    vao = 0;
    vbo = 0;
    poolShaders[0] = NULL;
    poolShaders[1] = NULL;
    poolShaders[2] = NULL;
}

OpenGLPool::~OpenGLPool()
{
    for(unsigned short i=0; i<3; ++i) if(poolShaders[i] != NULL) delete poolShaders[i];
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void OpenGLPool::InitPool()
{
    cInfo("Filling up the pool...");
    
	GLfloat surfData[4][3] = {{-1000.f, -1000.f, 0.f},
							  {1000.f,  -1000.f, 0.f},
							  {-1000.f, 1000.f, 0.f},
							  {1000.f,  1000.f, 0.f}};
							  
	
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo); 
	
    glBindVertexArray(vao);	
	glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(surfData), surfData, GL_STATIC_DRAW); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
    glBindVertexArray(0);
    
    //Load shaders
    poolShaders[0] = new GLSLShader("poolSurface.frag", "flat.vert");
	poolShaders[0]->AddUniform("MVP", ParameterType::MAT4);
	poolShaders[0]->AddUniform("texReflection", ParameterType::INT);
	
	poolShaders[1] = new GLSLShader("poolVolume.frag");
	poolShaders[1]->AddUniform("texScene", ParameterType::INT);
	poolShaders[1]->AddUniform("texLinearDepth", ParameterType::INT);
}	

void OpenGLPool::DrawPoolSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture)
{
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, reflectionTexture);
	
    poolShaders[0]->Use();
    poolShaders[0]->SetUniform("MVP", projection*view);
    poolShaders[0]->SetUniform("texReflection", TEX_POSTPROCESS1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
    
    glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
}

void OpenGLPool::DrawPoolBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection)
{
	glDisable(GL_CULL_FACE);
	//DrawPoolSurface(eyePos, view, projection);
	glEnable(GL_CULL_FACE);
}

void OpenGLPool::DrawPoolVolume(GLuint sceneTexture, GLuint linearDepthTex)
{
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, sceneTexture);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, linearDepthTex);
	
	poolShaders[1]->Use();
	poolShaders[1]->SetUniform("texScene", TEX_POSTPROCESS1);
	poolShaders[1]->SetUniform("texLinearDepth", TEX_POSTPROCESS2);
	OpenGLContent::getInstance()->DrawSAQ();
	glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
	
}