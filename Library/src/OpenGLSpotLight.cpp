//
//  OpenGLSpotLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSpotLight.h"
#include "SimulationManager.h"
#include "MathsUtil.hpp"

OpenGLSpotLight::OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, glm::vec4 color) : OpenGLLight(position, color)
{
    btVector3 dir = UnitSystem::SetPosition(target - position).normalized();
	direction = glm::vec3((GLfloat)dir.getX(), (GLfloat)dir.getY(), (GLfloat)dir.getZ());
	
    coneAngle = cone/180.f*M_PI;//UnitSystem::SetAngle(cone);
    clipSpace = glm::mat4();
    
    //Create shadowmap texture
    shadowSize = 2048;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowSize, shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //Create shadowmap framebuffer
    glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        printf("FBO initialization failed.\n");
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLSpotLight::~OpenGLSpotLight()
{
    if(shadowMap != 0)
        glDeleteTextures(1, &shadowMap);
    if(shadowFBO != 0)
        glDeleteFramebuffers(1, &shadowFBO);
}

LightType OpenGLSpotLight::getType()
{
	return SPOT_LIGHT;
}

glm::vec3 OpenGLSpotLight::getDirection()
{
    if(holdingEntity != NULL)
	{
		glm::mat4 trans = glMatrixFromBtTransform(holdingEntity->getTransform());
        return glm::mat3(trans) * direction;
	}
	else
        return direction;
}

GLfloat OpenGLSpotLight::getAngle()
{
    return coneAngle;
}

glm::mat4 OpenGLSpotLight::getClipSpace()
{
	return clipSpace;
}

void OpenGLSpotLight::SetupShader(GLSLShader* shader, unsigned int lightId)
{
	std::string lightUni = "spotLights[" + std::to_string(lightId) + "].";
	shader->SetUniform(lightUni + "position", getPosition());
	shader->SetUniform(lightUni + "color", getColor());
	shader->SetUniform(lightUni + "direction", getDirection());
	shader->SetUniform(lightUni + "angle", (GLfloat)cosf(getAngle()));
	shader->SetUniform(lightUni + "clipSpace", getClipSpace());
	shader->SetUniform(lightUni + "shadowMap", TEX_SHADOW_START + (int)lightId);
	
	glActiveTexture(GL_TEXTURE0 + TEX_SHADOW_START + lightId);
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
}

void OpenGLSpotLight::RenderDummy()
{
    //transformation
	glm::vec3 org = getPosition();
	glm::vec3 left = getDirection();
	glm::vec3 up(0,1,0);
	if(fabsf(left.y) > 0.8f)
		up = glm::vec3(0,0,1);
	
    glm::vec3 front = glm::normalize(glm::cross(left, up));
    up = glm::cross(front, left);
    
	glm::mat4 model(left.x, left.y, left.z, 0, 
				    up.x, up.y, up.z, 0,
				    front.x, front.y, front.z, 0,
				    org.x, org.y, org.z, 1);
	
    //rendering
    GLfloat iconSize = 5.f;
    int steps = 24;
    
    GLfloat r = iconSize*tanf(coneAngle);
    
    std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(iconSize, 0, 0));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(iconSize, r, 0));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(iconSize,-r, 0));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(iconSize, 0, r));
	vertices.push_back(glm::vec3(0,0,0));
	vertices.push_back(glm::vec3(iconSize, 0, -r));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR, model);
	vertices.clear();
	
	for(unsigned int i=0; i<=steps; ++i)
		vertices.push_back(glm::vec3(iconSize/2.f, cosf(i/(GLfloat)steps*2.f*M_PI)*r/2.f, sinf(i/(GLfloat)steps*2.f*M_PI)*r/2.f));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR, model);
	vertices.clear();
	
	for(unsigned int i=0; i<=steps; ++i)
		vertices.push_back(glm::vec3(iconSize, cosf(i/(GLfloat)steps*2.f*M_PI)*r, sinf(i/(GLfloat)steps*2.f*M_PI)*r));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR, model);
}

void OpenGLSpotLight::RenderShadowMap(OpenGLPipeline* pipe, SimulationManager* sim)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glm::mat4 proj = glm::perspective((GLfloat)(2.f * coneAngle), 1.f, 1.f, 100.f);
    glm::mat4 view = glm::lookAt(getPosition(),
                                 getPosition() + getDirection(),
                                 glm::vec3(0,0,1.f));
    
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    clipSpace = bias * (proj * view);
	
	OpenGLContent::getInstance()->SetProjectionMatrix(proj);
	OpenGLContent::getInstance()->SetViewMatrix(view);
	
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, shadowSize, shadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
	pipe->DrawObjects(sim);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLSpotLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	OpenGLContent::getInstance()->DrawTexturedQuad(x, y, w, h, shadowMap);
}


