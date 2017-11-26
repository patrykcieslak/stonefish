//
//  OpenGLPool.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPool.h"
#include "Console.h"
#include "OpenGLAtmosphere.h"
#include "SystemUtil.hpp"

OpenGLPool::OpenGLPool()
{
    vao = 0;
    vbo = 0;
    //poolShaders[0] = NULL;
    //poolShaders[1] = NULL;
    //poolShaders[2] = NULL;
	//poolShaders[3] = NULL;
	t = 0;
    lightAbsorption = glm::vec3(0.f);
	turbidity = 0.f;
}

OpenGLPool::~OpenGLPool()
{
    //for(unsigned short i=0; i<4; ++i) if(poolShaders[i] != NULL) delete poolShaders[i];
	for(unsigned int i=0; i<poolShaders.size(); ++i) delete poolShaders[i];
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void OpenGLPool::setLightAbsorptionCoeff(glm::vec3 a)
{
    lightAbsorption = a;
}

void OpenGLPool::setTurbidity(GLfloat tur)
{
	turbidity = tur < 1.f ? 1.f : tur;
}

glm::vec3 OpenGLPool::getLightAbsorptionCoeff()
{
    return lightAbsorption;
}

GLfloat OpenGLPool::getTurbidity()
{
	return turbidity;
}

void OpenGLPool::Init()
{
    cInfo("Filling up the pool...");
    
	GLfloat surfData[12][4] = {{0.f, 0.f,  0.f, 1.f},
							  {1.f, 0.f,  0.f, 0.f},
							  {0.f, 1.f,  0.f, 0.f},
							  {0.f, 0.f,  0.f, 1.f},
							  {0.f, 1.f,  0.f, 0.f},
							  {-1.f, 0.f, 0.f, 0.f},
							  {0.f, 0.f,  0.f, 1.f},
							  {-1.f, 0.f, 0.f, 0.f},
							  {0.f, -1.f, 0.f, 0.f},
							  {0.f, 0.f,  0.f, 1.f},
							  {0.f, -1.f, 0.f, 0.f},
							  {1.f, 0.f,  0.f, 0.f}};
							  
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo); 
	
    glBindVertexArray(vao);	
	glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(surfData), surfData, GL_STATIC_DRAW); 
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
    glBindVertexArray(0);
    
    //Load shaders
	std::vector<GLuint> precompiled;
	precompiled.push_back(OpenGLAtmosphere::getInstance()->getAtmosphereAPI());
	
	GLSLShader* poolSurface = new GLSLShader(precompiled, "poolSurface.frag", "poolSurface.vert");
	poolSurface->AddUniform("MVP", ParameterType::MAT4);
	poolSurface->AddUniform("viewport", ParameterType::VEC2);
	poolSurface->AddUniform("eyePos", ParameterType::VEC3);
	poolSurface->AddUniform("R0", ParameterType::FLOAT);
	poolSurface->AddUniform("time", ParameterType::FLOAT);
	poolSurface->AddUniform("texReflection", ParameterType::INT);
	poolSurface->AddUniform("transmittance_texture", ParameterType::INT);
	poolSurface->AddUniform("scattering_texture", ParameterType::INT);
	poolSurface->AddUniform("irradiance_texture", ParameterType::INT);
	poolSurface->AddUniform("sunRadius", ParameterType::VEC3);
	poolSurface->AddUniform("planetRadius", ParameterType::FLOAT);
	poolSurface->AddUniform("whitePoint", ParameterType::VEC3);
	poolSurface->AddUniform("sunDirection", ParameterType::VEC3);
	poolSurface->AddUniform("cosSunSize", ParameterType::FLOAT);
	poolShaders.push_back(poolSurface);
	
	GLSLShader* poolBacksurface = new GLSLShader(precompiled, "poolBacksurface.frag", "poolSurface.vert");
	poolBacksurface->AddUniform("MVP", ParameterType::MAT4);
	poolBacksurface->AddUniform("viewport", ParameterType::VEC2);
	poolBacksurface->AddUniform("eyePos", ParameterType::VEC3);
	poolBacksurface->AddUniform("R0", ParameterType::FLOAT);
	poolBacksurface->AddUniform("time", ParameterType::FLOAT);
    poolBacksurface->AddUniform("lightAbsorption", ParameterType::VEC3);
	poolBacksurface->AddUniform("turbidity", ParameterType::FLOAT);
	poolBacksurface->AddUniform("texReflection", ParameterType::INT);
	poolBacksurface->AddUniform("transmittance_texture", ParameterType::INT);
	poolBacksurface->AddUniform("scattering_texture", ParameterType::INT);
	poolBacksurface->AddUniform("irradiance_texture", ParameterType::INT);
	poolBacksurface->AddUniform("sunRadius", ParameterType::VEC3);
	poolBacksurface->AddUniform("planetRadius", ParameterType::FLOAT);
	poolBacksurface->AddUniform("whitePoint", ParameterType::VEC3);
	poolBacksurface->AddUniform("sunDirection", ParameterType::VEC3);
	poolBacksurface->AddUniform("cosSunSize", ParameterType::FLOAT);
	poolShaders.push_back(poolBacksurface);
	
	GLSLShader* poolBackground = new GLSLShader(precompiled, "poolBackground.frag", "saq.vert");
    poolBackground->AddUniform("lightAbsorption", ParameterType::VEC3);
	poolBackground->AddUniform("eyePos", ParameterType::VEC3);
	poolBackground->AddUniform("invProj", ParameterType::MAT4);
	poolBackground->AddUniform("invView", ParameterType::MAT3);
	poolBackground->AddUniform("transmittance_texture", ParameterType::INT);
	poolBackground->AddUniform("scattering_texture", ParameterType::INT);
	poolBackground->AddUniform("irradiance_texture", ParameterType::INT);
	poolBackground->AddUniform("sunRadius", ParameterType::VEC3);
	poolBackground->AddUniform("planetRadius", ParameterType::FLOAT);
	poolBackground->AddUniform("whitePoint", ParameterType::VEC3);
	poolBackground->AddUniform("sunDirection", ParameterType::VEC3);
	poolBackground->AddUniform("cosSunSize", ParameterType::FLOAT);
	poolShaders.push_back(poolBackground);
	
	GLSLShader* poolVolume = new GLSLShader("poolVolume.frag");
	poolVolume->AddUniform("texScene", ParameterType::INT);
	poolVolume->AddUniform("texLinearDepth", ParameterType::INT);
	poolShaders.push_back(poolVolume);
}	

void OpenGLPool::DrawSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport)
{
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, reflectionTexture);
	
	GLfloat na = 1.f;
	GLfloat nw = 1.33f; 
	GLfloat R0 = (na-nw)*(na-nw)/((na+nw)*(na+nw));
    poolShaders[0]->Use();
    poolShaders[0]->SetUniform("MVP", projection*view);
	poolShaders[0]->SetUniform("texReflection", TEX_POSTPROCESS1);
	poolShaders[0]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
	poolShaders[0]->SetUniform("eyePos", eyePos);
	poolShaders[0]->SetUniform("R0", R0);
	poolShaders[0]->SetUniform("time", t);
    OpenGLAtmosphere::getInstance()->SetupOceanShader(poolShaders[0]);
	t+= 0.01;
	
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    
    glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
}

void OpenGLPool::DrawBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport)
{
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, reflectionTexture);
	GLfloat na = 1.33f;
	GLfloat nw = 1.0f; 
	GLfloat R0 = (na-nw)*(na-nw)/((na+nw)*(na+nw));
    poolShaders[1]->Use();
    poolShaders[1]->SetUniform("MVP", projection*view);
	poolShaders[1]->SetUniform("texReflection", TEX_POSTPROCESS1);
	poolShaders[1]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
	poolShaders[1]->SetUniform("eyePos", eyePos);
	poolShaders[1]->SetUniform("R0", R0);
	poolShaders[1]->SetUniform("time", t);
    poolShaders[1]->SetUniform("lightAbsorption", lightAbsorption);
	poolShaders[1]->SetUniform("turbidity", turbidity);
    OpenGLAtmosphere::getInstance()->SetupOceanShader(poolShaders[1]);
	t+= 0.01;
	
	glDisable(GL_CULL_FACE);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
    
    glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
}

void OpenGLPool::DrawBackground(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection)
{
	poolShaders[2]->Use();
    poolShaders[2]->SetUniform("lightAbsorption", lightAbsorption);
	poolShaders[2]->SetUniform("eyePos", eyePos);
	poolShaders[2]->SetUniform("invProj", glm::inverse(projection));
	poolShaders[2]->SetUniform("invView", glm::inverse(glm::mat3(view)));
    OpenGLAtmosphere::getInstance()->SetupOceanShader(poolShaders[2]);
	OpenGLContent::getInstance()->DrawSAQ();
	glUseProgram(0);
}

void OpenGLPool::DrawVolume(GLuint sceneTexture, GLuint linearDepthTex)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, sceneTexture);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, linearDepthTex);
	
	poolShaders[3]->Use();
	poolShaders[3]->SetUniform("texScene", TEX_POSTPROCESS1);
	poolShaders[3]->SetUniform("texLinearDepth", TEX_POSTPROCESS2);
	OpenGLContent::getInstance()->DrawSAQ();
	glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
	
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
}