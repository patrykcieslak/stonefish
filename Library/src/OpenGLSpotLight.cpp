//
//  OpenGLSpotLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSpotLight.h"
#include "SimulationManager.h"

OpenGLSpotLight::OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, glm::vec4 color) : OpenGLLight(position, color)
{
    dir = UnitSystem::SetPosition(target - position).normalized();
    coneAngle = cone/180.f*M_PI;//UnitSystem::SetAngle(cone);
    lightClipSpace = glm::mat4();
    
    //Create shadowmap texture
    shadowSize = 2048;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowSize, shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
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

btVector3 OpenGLSpotLight::getViewDirection()
{
    btVector3 direction = (activeView->GetViewTransform()).getBasis() * getDirection();
    return direction;
}

btVector3 OpenGLSpotLight::getDirection()
{
    if(holdingEntity != NULL)
        return holdingEntity->getTransform().getBasis() * dir;
    else
        return dir;
}

GLfloat OpenGLSpotLight::getAngle()
{
    return coneAngle;
}

void OpenGLSpotLight::Render()
{
    if(isActive())
    {
        btVector3 lightPos = getViewPosition();
        glm::vec3 lposition((GLfloat)lightPos.getX(), (GLfloat)lightPos.getY(), (GLfloat)lightPos.getZ());
        btVector3 lightDir = getViewDirection();
        glm::vec3 ldirection((GLfloat)lightDir.getX(), (GLfloat)lightDir.getY(), (GLfloat)lightDir.getZ());
        glm::mat4 eyeToLight = lightClipSpace * glm::inverse(activeView->GetViewMatrix(activeView->GetViewTransform()));

        glActiveTexture(GL_TEXTURE0 + shadowTextureUnit);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        
        spotShader->Enable();
        spotShader->SetUniform("texDiffuse", diffuseTextureUnit);
        spotShader->SetUniform("texPosition", positionTextureUnit);
        spotShader->SetUniform("texNormal", normalTextureUnit);
        spotShader->SetUniform("texShadow", shadowTextureUnit);
        spotShader->SetUniform("lightPosition", lposition);
        spotShader->SetUniform("lightDirection", ldirection);
        spotShader->SetUniform("lightAngle", (GLfloat)cosf(getAngle()));
        spotShader->SetUniform("lightColor", getColor());
        spotShader->SetUniform("lightClipSpace", eyeToLight);
        OpenGLContent::getInstance()->DrawSAQ();
        spotShader->Disable();
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void OpenGLSpotLight::RenderDummy()
{
    //transformation
    btVector3 org = getPosition();
    btVector3 left = getDirection();
    
    btVector3 up = btVector3(0, 1.0, 0);
    if(fabs(left.y()) > 0.8)
        up = btVector3(0,0,1.0);
    
    btVector3 front = left.cross(up);
    front.normalize();
    up = front.cross(left);
    
	glm::mat4 model((GLfloat)left.x(), (GLfloat)left.y(), (GLfloat)left.z(), 0, 
				   (GLfloat)up.x(), (GLfloat)up.y(), (GLfloat)up.z(), 0,
				   (GLfloat)front.x(), (GLfloat)front.y(), (GLfloat)front.z(), 0,
				   (GLfloat)org.x(), (GLfloat)org.y(), (GLfloat)org.z(), 1.f);
	
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
	btVector3 lightPos = getPosition();
    btVector3 lightDir = dir;
    glm::mat4 view = glm::lookAt(glm::vec3(lightPos.x(), lightPos.y(), lightPos.z()),
                                 glm::vec3(lightPos.x() + lightDir.x(), lightPos.y() + lightDir.y(), lightPos.z() + lightDir.z()),
                                 glm::vec3(0,0,1.f));
    
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    lightClipSpace = bias * (proj * view);
	
	OpenGLContent::getInstance()->SetProjectionMatrix(proj);
	OpenGLContent::getInstance()->SetViewMatrix(view);
	
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, shadowSize, shadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
	pipe->DrawObjects(sim);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glCullFace(GL_BACK);
}

void OpenGLSpotLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	OpenGLContent::getInstance()->DrawTexturedQuad(x, y, w, h, shadowMap);
}


