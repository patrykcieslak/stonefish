//
//  OpenGLSpotLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSpotLight.h"

OpenGLSpotLight::OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, GLfloat* color4) : OpenGLLight(position, color4)
{
    reldir = UnitSystem::SetPosition(target-position);
    reldir.normalize();
    dir = reldir;
    coneAngle = UnitSystem::SetAngle(cone);
}

OpenGLSpotLight::~OpenGLSpotLight()
{
}

btVector3 OpenGLSpotLight::getDirection()
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
    UseSpotShader(this);
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

void OpenGLSpotLight::UseSpotShader(OpenGLSpotLight* light)
{
	btVector3 lightPos = light->getPosition();
	GLfloat lposition[3] = {(GLfloat)lightPos.getX(),(GLfloat)lightPos.getY(),(GLfloat)lightPos.getZ()};
	btVector3 lightDir = light->getDirection();
	GLfloat ldirection[3] = {(GLfloat)lightDir.getX(),(GLfloat)lightDir.getY(),(GLfloat)lightDir.getZ()};
    
    glUseProgramObjectARB(spotShader);
    glUniform1iARB(uniSDiffuse, diffuseTextureUnit);
    glUniform1iARB(uniSPosition, positionTextureUnit);
    glUniform1iARB(uniSNormal, normalTextureUnit);
    glUniform3fvARB(uniSLightPos, 1, lposition);
    glUniform3fvARB(uniSLightDir, 1, ldirection);
    glUniform1f(uniSLightAngle, cosf(light->getAngle()));
    glUniform4fvARB(uniSColor, 1, light->getColor());
}
