//
//  OpenGLOmniLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLOmniLight.h"
#include "SimulationManager.h"

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
	/*
    //transformation
    btTransform trans(btQuaternion::getIdentity(), getPosition());
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
    glMultMatrixd(openglTrans);
#else
    glMultMatrixf(openglTrans);
#endif
    
    //rendering
    GLfloat iconSize = surfaceDistance;
    int steps = 24;
    
    glColor4f(0.f, 1.f, 0.f, 1.f);
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<steps; i++)
    {
        glVertex3f(sinf(i/(GLfloat)steps*2.f*M_PI)*iconSize, cosf(i/(GLfloat)steps*2.f*M_PI)*iconSize, 0.f);
    }
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<steps; i++)
    {
        glVertex3f(0.f, cosf(i/(GLfloat)steps*2.f*M_PI)*iconSize, sinf(i/(GLfloat)steps*2.f*M_PI)*iconSize);
    }
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<steps; i++)
    {
        glVertex3f(sinf(i/(GLfloat)steps*2.f*M_PI)*iconSize, 0.f, cosf(i/(GLfloat)steps*2.f*M_PI)*iconSize);
    }
    glEnd();
    
    glPopMatrix();
	 */
}

void OpenGLOmniLight::RenderShadowMap(OpenGLPipeline* pipe, SimulationManager* sim)
{
    
}

void OpenGLOmniLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat scale)
{
    
}
