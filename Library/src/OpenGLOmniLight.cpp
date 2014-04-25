//
//  OpenGLOmniLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLOmniLight.h"
#include "OpenGLSolids.h"

OpenGLOmniLight::OpenGLOmniLight(const btVector3& position, GLfloat* color4) : OpenGLLight(position, color4)
{
}

OpenGLOmniLight::~OpenGLOmniLight()
{
}

void OpenGLOmniLight::UpdateLight()
{
    if(holdingEntity != NULL)
    {
        btTransform trans;
        trans = holdingEntity->getTransform();
        btQuaternion rotation = trans.getRotation();
        btTransform rotTrans(rotation, btVector3(0,0,0));
        pos = rotTrans * relpos + trans.getOrigin();
    }
}

void OpenGLOmniLight::Render()
{
    if(isActive())
    {
        btVector3 lightPos = getViewPosition();
        GLfloat lposition[3] = {(GLfloat)lightPos.getX(),(GLfloat)lightPos.getY(),(GLfloat)lightPos.getZ()};
        
        glUseProgramObjectARB(omniShader);
        glUniform1iARB(uniODiffuse, diffuseTextureUnit);
        glUniform1iARB(uniOPosition, positionTextureUnit);
        glUniform1iARB(uniONormal, normalTextureUnit);
        glUniform3fvARB(uniOLightPos, 1, lposition);
        glUniform4fvARB(uniOColor, 1, getColor());
        DrawScreenAlignedQuad();
        glUseProgramObjectARB(0);
    }
}

void OpenGLOmniLight::RenderLightSurface()
{
    if(surfaceDistance > 0)
    {
        //transformation
        btScalar openglTrans[16];
        openglTrans[0] = 1;
        openglTrans[1] = 0;
        openglTrans[2] = 0;
        openglTrans[3] = 0;
        openglTrans[4] = 0;
        openglTrans[5] = 1;
        openglTrans[6] = 0;
        openglTrans[7] = 0;
        openglTrans[8] = 0;
        openglTrans[9] = 0;
        openglTrans[10] = 1;
        openglTrans[11] = 0;
        openglTrans[12] = pos.x();
        openglTrans[13] = pos.y();
        openglTrans[14] = pos.z();
        openglTrans[15] = 1;
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        //rendering
        glColor4f(color[0], color[1], color[2], 1.0f);
        DrawSolidSphere(surfaceDistance);
        
        glPopMatrix();
    }
}

void OpenGLOmniLight::RenderDummy()
{
    //transformation
    btScalar openglTrans[16];
    openglTrans[0] = 1;
    openglTrans[1] = 0;
    openglTrans[2] = 0;
    openglTrans[3] = 0;
    openglTrans[4] = 0;
    openglTrans[5] = 1;
    openglTrans[6] = 0;
    openglTrans[7] = 0;
    openglTrans[8] = 0;
    openglTrans[9] = 0;
    openglTrans[10] = 1;
    openglTrans[11] = 0;
    openglTrans[12] = pos.x();
    openglTrans[13] = pos.y();
    openglTrans[14] = pos.z();
    openglTrans[15] = 1;
    
    glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
    glMultMatrixd(openglTrans);
#else
    glMultMatrixf(openglTrans);
#endif
    
    //rendering
    GLfloat iconSize = 5.f;
    int steps = 24;
    
    glColor4f(0.f, 1.f, 0.f, 1.f);
    
    iconSize /= 3.f;
    
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
}

void OpenGLOmniLight::RenderShadowMap(OpenGLPipeline* pipe)
{
    
}

void OpenGLOmniLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat scale)
{
    
}
