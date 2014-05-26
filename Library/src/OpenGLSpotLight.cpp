//
//  OpenGLSpotLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSpotLight.h"
#include "OpenGLSolids.h"

OpenGLSpotLight::OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, glm::vec4 color) : OpenGLLight(position, color)
{
    reldir = UnitSystem::SetPosition(target - position);
    reldir.normalize();
    dir = reldir;
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
    btVector3 direction = (activeView->GetViewTransform()).getBasis() * dir;
    return direction;
}

GLfloat OpenGLSpotLight::getAngle()
{
    return coneAngle;
}

void OpenGLSpotLight::UpdateLight()
{
    if(holdingEntity != NULL)
    {
        btTransform trans;
        trans = holdingEntity->getTransform();
        btQuaternion rotation = trans.getRotation();
        btTransform rotTrans(rotation, btVector3(0,0,0));
        dir = rotTrans * reldir;
        pos = rotTrans * relpos + trans.getOrigin();
    }
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

        glActiveTextureARB(GL_TEXTURE0_ARB + shadowTextureUnit);
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
        OpenGLSolids::DrawScreenAlignedQuad();
        spotShader->Disable();
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void OpenGLSpotLight::RenderLightSurface()
{
    if(surfaceDistance > 0)
    {
        //transformation
        btVector3 up = btVector3(0, 1.0, 0);
        btVector3 left = dir.normalized();
		if(fabs(left.y())>0.8)
            up = btVector3(0,0,1.0);
        btVector3 front = left.cross(up);
        front.normalize();
        up = front.cross(left);
        
        btScalar openglTrans[16];
        openglTrans[0] = left.x();
        openglTrans[1] = left.y();
        openglTrans[2] = left.z();
        openglTrans[3] = 0.0;
        openglTrans[4] = up.x();
        openglTrans[5] = up.y();
        openglTrans[6] = up.z();
        openglTrans[7] = 0.0;
        openglTrans[8] = front.x();
        openglTrans[9] = front.y();
        openglTrans[10] = front.z();
        openglTrans[11] = 0.0;
        openglTrans[12] = pos.x();
        openglTrans[13] = pos.y();
        openglTrans[14] = pos.z();
        openglTrans[15] = 1.0;
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        
        //rendering
        int steps = 24;
        glColor4f(color[0], color[1], color[2], 1.0f);
        
        GLfloat r = surfaceDistance*tanf(coneAngle);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(surfaceDistance,0,0);
        for(int i=0; i<=steps; i++)
            glVertex3f(surfaceDistance, cosf(i/(GLfloat)steps*2.f*M_PI)*r, sinf(i/(GLfloat)steps*2.f*M_PI)*r);
        glEnd();
        
        glPopMatrix();
    }
}

void OpenGLSpotLight::RenderDummy()
{
    //transformation
    btVector3 up = btVector3(0, 1.0, 0);
    btVector3 left = dir.normalized();
	if(fabs(left.y())>0.8)
        up = btVector3(0,0,1.0);
    btVector3 front = left.cross(up);
    front.normalize();
    up = front.cross(left);
    
    btScalar openglTrans[16];
    openglTrans[0] = left.x();
    openglTrans[1] = left.y();
    openglTrans[2] = left.z();
    openglTrans[3] = 0.0;
    openglTrans[4] = up.x();
    openglTrans[5] = up.y();
    openglTrans[6] = up.z();
    openglTrans[7] = 0.0;
    openglTrans[8] = front.x();
    openglTrans[9] = front.y();
    openglTrans[10] = front.z();
    openglTrans[11] = 0.0;
    openglTrans[12] = pos.x();
    openglTrans[13] = pos.y();
    openglTrans[14] = pos.z();
    openglTrans[15] = 1.0;
    
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
    
    GLfloat r = iconSize*tanf(coneAngle);
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(iconSize, 0 ,0);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(iconSize, r, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(iconSize, -r, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(iconSize, 0, r);
    glVertex3f(0, 0, 0);
    glVertex3f(iconSize, 0, -r);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<steps; i++)
        glVertex3f(iconSize/2.f, cosf(i/(GLfloat)steps*2.f*M_PI)*r/2.f, sinf(i/(GLfloat)steps*2.f*M_PI)*r/2.f);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<steps; i++)
        glVertex3f(iconSize, cosf(i/(GLfloat)steps*2.f*M_PI)*r, sinf(i/(GLfloat)steps*2.f*M_PI)*r);
    glEnd();
    
    glPopMatrix();
}

void OpenGLSpotLight::RenderShadowMap(OpenGLPipeline* pipe)
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glm::mat4 proj = glm::perspective((GLfloat)(2.f * coneAngle / M_PI * 180.f), 1.f, 1.f, 100.f);
    glLoadMatrixf(glm::value_ptr(proj));
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    btVector3 lightPos = getPosition();
    btVector3 lightDir = dir;
    glm::mat4 view = glm::lookAt(glm::vec3(lightPos.x(), lightPos.y(), lightPos.z()),
                                 glm::vec3(lightPos.x() + lightDir.x(), lightPos.y() + lightDir.y(), lightPos.z() + lightDir.z()),
                                 glm::vec3(0,0,1.f));
    
    glLoadMatrixf(glm::value_ptr(view));
    
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    lightClipSpace = bias * (proj * view);
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, shadowSize, shadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    pipe->DrawStandardObjects();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glCullFace(GL_BACK);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void OpenGLSpotLight::ShowShadowMap(GLfloat x, GLfloat y, GLfloat scale)
{
    //save current state
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glPushAttrib(GL_VIEWPORT_BIT);
    
    //Set projection and modelview
    OpenGLSolids::SetupOrtho();
    
	//Texture setup
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    
	//Render the shadowmap
    glViewport(x, y, shadowSize * scale, shadowSize * scale);
    glColor4f(1.f, 1.f, 1.f, 1.f);
    OpenGLSolids::DrawScreenAlignedQuad();
    
	//Reset
	glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


