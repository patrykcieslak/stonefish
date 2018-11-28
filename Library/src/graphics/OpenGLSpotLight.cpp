//
//  OpenGLSpotLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSpotLight.h"

#include "core/SimulationManager.h"
#include "utils/MathUtil.hpp"

using namespace sf;

OpenGLSpotLight::OpenGLSpotLight(const Vector3& position, const Vector3& target, GLfloat cone, glm::vec4 color) : OpenGLLight(position, color)
{
    Vector3 dir = (target - position).normalized();
	direction = glm::vec3((GLfloat)dir.getX(), (GLfloat)dir.getY(), (GLfloat)dir.getZ());
	
    coneAngle = cone/180.f*M_PI;
    clipSpace = glm::mat4();
	zNear = 0.1f;
	zFar = 100.f;
}

OpenGLSpotLight::~OpenGLSpotLight()
{
    if(shadowFBO != 0) glDeleteFramebuffers(1, &shadowFBO);
}

void OpenGLSpotLight::InitShadowmap(GLint shadowmapLayer)
{
	//Create shadowmap framebuffer
    glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, spotShadowArrayTex, 0, shadowmapLayer);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        printf("FBO initialization failed.\n");
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

LightType OpenGLSpotLight::getType()
{
	return SPOT_LIGHT;
}

glm::vec3 OpenGLSpotLight::getDirection()
{
    if(holdingEntity != NULL)
	{
		glm::mat4 trans = glMatrixFromBtTransform(holdingEntity->getCGTransform());
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
	shader->SetUniform(lightUni + "radius", glm::vec2(0.01f,0.01f));
	shader->SetUniform(lightUni + "color", getColor());
	shader->SetUniform(lightUni + "direction", getDirection());
	shader->SetUniform(lightUni + "angle", (GLfloat)cosf(getAngle()));
	shader->SetUniform(lightUni + "clipSpace", getClipSpace());
	shader->SetUniform(lightUni + "zNear", zNear);
	shader->SetUniform(lightUni + "zFar", zFar);
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
    unsigned int steps = 24;
    
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

void OpenGLSpotLight::BakeShadowmap(OpenGLPipeline* pipe)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glm::mat4 proj = glm::perspective((GLfloat)(2.f * coneAngle), 1.f, zNear, zFar);
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
    glViewport(0, 0, SPOT_LIGHT_SHADOWMAP_SIZE, SPOT_LIGHT_SHADOWMAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(4.0f, 32.0f);
	pipe->DrawObjects();
	//glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLSpotLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	/*glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	OpenGLContent::getInstance()->DrawTexturedQuad(x, y, w, h, shadowMap);*/
}


