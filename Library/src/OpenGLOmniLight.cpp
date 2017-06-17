//
//  OpenGLOmniLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLOmniLight.h"
#include "SimulationManager.h"
#include "GeometryUtil.hpp"

OpenGLOmniLight::OpenGLOmniLight(const btVector3& position, glm::vec4 color) : OpenGLLight(position, color)
{
}

OpenGLOmniLight::~OpenGLOmniLight()
{
}

void OpenGLOmniLight::Render()
{
    if(isActive())
    {
        btVector3 lightPos = getViewPosition();
        glm::vec3 lpos((GLfloat)lightPos.getX(),(GLfloat)lightPos.getY(),(GLfloat)lightPos.getZ());
        
        omniShader->Enable();
        omniShader->SetUniform("texDiffuse", diffuseTextureUnit);
        omniShader->SetUniform("texPosition", positionTextureUnit);
        omniShader->SetUniform("texNormal", normalTextureUnit);
        omniShader->SetUniform("lightPosition", lpos);
        omniShader->SetUniform("lightColor", getColor());
        OpenGLContent::getInstance()->DrawSAQ();
        omniShader->Disable();
    }
}

void OpenGLOmniLight::RenderDummy()
{
    btTransform trans(btQuaternion::getIdentity(), getPosition());
    glm::mat4 model = glMatrixFromBtTransform(trans);
	
    GLfloat iconSize = surfaceDistance;
    int steps = 24;
    
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

void OpenGLOmniLight::RenderShadowMap(OpenGLPipeline* pipe, SimulationManager* sim)
{
    
}

void OpenGLOmniLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    
}
