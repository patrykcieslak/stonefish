//
//  OpenGLPointLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLPointLight.h"

#include "core/SimulationManager.h"
#include "utils/MathUtil.hpp"

using namespace sf;

OpenGLPointLight::OpenGLPointLight(const btVector3& position, glm::vec4 color) : OpenGLLight(position, color)
{
}

OpenGLPointLight::~OpenGLPointLight()
{
}

LightType OpenGLPointLight::getType()
{
	return POINT_LIGHT;
}

void OpenGLPointLight::InitShadowmap(GLint shadowmapLayer)
{
}

void OpenGLPointLight::SetupShader(GLSLShader* shader, unsigned int lightId)
{
	std::string lightUni = "pointLights[" + std::to_string(lightId) + "].";
	shader->SetUniform(lightUni + "position", getPosition());
	shader->SetUniform(lightUni + "color", getColor());
}

void OpenGLPointLight::RenderDummy()
{
    glm::mat4 model = glm::translate(getPosition());
    GLfloat iconSize = surfaceDistance;
    unsigned int steps = 24;
    
	std::vector<glm::vec3> vertices;
	
	for(unsigned int i=0; i<=steps; ++i)
		vertices.push_back(glm::vec3(sinf(i/(GLfloat)steps*2.f*M_PI)*iconSize, cosf(i/(GLfloat)steps*2.f*M_PI)*iconSize, 0.f));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR, model);
	vertices.clear();
	
	for(unsigned int i=0; i<=steps; ++i)
		vertices.push_back(glm::vec3(0.f, cosf(i/(GLfloat)steps*2.f*M_PI)*iconSize, sinf(i/(GLfloat)steps*2.f*M_PI)*iconSize));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR, model);
	vertices.clear();
	
	for(unsigned int i=0; i<=steps; ++i)
		vertices.push_back(glm::vec3(sinf(i/(GLfloat)steps*2.f*M_PI)*iconSize, 0.f, cosf(i/(GLfloat)steps*2.f*M_PI)*iconSize));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR, model);
}

void OpenGLPointLight::BakeShadowmap(OpenGLPipeline* pipe)
{
    
}

void OpenGLPointLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    
}
