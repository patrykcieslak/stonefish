//
//  OpenGLSolids.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSolids.h"
#include <math.h>

GLint OpenGLSolids::saqDisplayList = 0;

void OpenGLSolids::Init()
{
    saqDisplayList = glGenLists(1);
    glNewList(saqDisplayList, GL_COMPILE);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.f, 0);
    glVertex3f(1.f, -1.0f, 0.0f);
    glTexCoord2f(0, 1.f);
    glVertex3f(-1.0f, 1.f, 0.0f);
    glTexCoord2f(1.f, 1.f);
    glVertex3f(1.f, 1.f, 0.0f);
    glEnd();
    glEndList();
}

void OpenGLSolids::Destroy()
{
    if(saqDisplayList != 0)
        glDeleteLists(saqDisplayList, 1);
}

void OpenGLSolids::SetupOrtho()
{
    glMatrixMode(GL_PROJECTION);
    glm::mat4 proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    glLoadMatrixf(glm::value_ptr(proj));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void OpenGLSolids::DrawScreenAlignedQuad()
{
    glCallList(saqDisplayList);
}

void OpenGLSolids::DrawCoordSystem(GLfloat size)
{
    glBegin(GL_LINES);
    glXAxisColor();
    glVertex3f(0, 0, 0);
    glVertex3f(size, 0, 0);
    
    glYAxisColor();
    glVertex3f(0, 0, 0);
    glVertex3f(0, size, 0);
    
    glZAxisColor();
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, size);
    glEnd();
}

void OpenGLSolids::DrawPoint(GLfloat size)
{
    glBegin(GL_LINES);
    glVertex3f(-size, 0, 0);
    glVertex3f(size, 0, 0);
    glVertex3f(0, -size, 0);
    glVertex3f(0, size, 0);
    glVertex3f(0, 0, -size);
    glVertex3f(0, 0, size);
    glEnd();
}

void OpenGLSolids::DrawSolidBox(GLfloat halfX, GLfloat halfY, GLfloat halfZ)
{
    glBegin(GL_TRIANGLES);
    //front
    glNormal3f(0, 0, -1);
    glVertex3f(-halfX, -halfY, -halfZ);
    glVertex3f(-halfX, halfY, -halfZ);
    glVertex3f(halfX, halfY, -halfZ);
    glVertex3f(-halfX, -halfY, -halfZ);
    glVertex3f(halfX, halfY, -halfZ);
    glVertex3f(halfX, -halfY, -halfZ);
    
    //left
    glNormal3f(1, 0, 0);
    glVertex3f(halfX, -halfY, -halfZ);
    glVertex3f(halfX, halfY, -halfZ);
    glVertex3f(halfX, halfY, halfZ);
    glVertex3f(halfX, -halfY, -halfZ);
    glVertex3f(halfX, halfY, halfZ);
    glVertex3f(halfX, -halfY, halfZ);
    
    //right
    glNormal3f(-1, 0, 0);
    glVertex3f(-halfX, -halfY, halfZ);
    glVertex3f(-halfX, halfY, halfZ);
    glVertex3f(-halfX, halfY, -halfZ);
    glVertex3f(-halfX, -halfY, halfZ);
    glVertex3f(-halfX, halfY, -halfZ);
    glVertex3f(-halfX, -halfY, -halfZ);
    
    //back
    glNormal3f(0, 0, 1);
    glVertex3f(halfX, -halfY, halfZ);
    glVertex3f(halfX, halfY, halfZ);
    glVertex3f(-halfX, halfY, halfZ);
    glVertex3f(halfX, -halfY, halfZ);
    glVertex3f(-halfX, halfY, halfZ);
    glVertex3f(-halfX, -halfY, halfZ);
    
    //top
    glNormal3f(0, 1, 0);
    glVertex3f(halfX, halfY, halfZ);
    glVertex3f(halfX, halfY, -halfZ);
    glVertex3f(-halfX, halfY, -halfZ);
    glVertex3f(halfX, halfY, halfZ);
    glVertex3f(-halfX, halfY, -halfZ);
    glVertex3f(-halfX, halfY, halfZ);
    
    //bottom
    glNormal3f(0, -1, 0);
    glVertex3f(halfX, -halfY, -halfZ);
    glVertex3f(halfX, -halfY, halfZ);
    glVertex3f(-halfX, -halfY, halfZ);
    glVertex3f(halfX, -halfY, -halfZ);
    glVertex3f(-halfX, -halfY, halfZ);
    glVertex3f(-halfX, -halfY, -halfZ);
    glEnd();
}

void OpenGLSolids::DrawSolidSphere(GLfloat radius)
{
    for(int i = -(SPHERE_RESOLUTION/4-1); i<(SPHERE_RESOLUTION/4-1); i++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for(int h = 0; h <= SPHERE_RESOLUTION; h++)
        {
            glNormal3f(cosf(i/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * cosf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI), sinf(i/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2), cosf(i/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * sinf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI));
            glVertex3f(cosf(i/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * cosf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius, sinf(i/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*radius, cosf(i/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*sinf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius);
            
            glNormal3f(cosf((i+1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*cosf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI), sinf((i+1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2), cosf((i+1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*sinf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI));
            glVertex3f(cosf((i+1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*cosf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius, sinf((i+1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*radius, cosf((i+1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*sinf(h/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius);
        }
        glEnd();
    }
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1.f, 0);
    glVertex3f(0, -radius, 0);
    for(int i=0; i<=SPHERE_RESOLUTION; i++)
    {
        glNormal3f(cosf(-(GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * cosf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI), sinf(-(GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2), cosf(-(GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*sinf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI));
        glVertex3f(cosf(-(GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)* cosf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius, sinf(-(GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*radius, cosf(-(GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * sinf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1.f, 0);
    glVertex3f(0, radius, 0);
    for(int i=SPHERE_RESOLUTION; i>=0; i--)
    {
        glNormal3f(cosf((GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * cosf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI), sinf((GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2), cosf((GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * sinf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI));
        glVertex3f(cosf((GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2) * cosf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius, sinf((GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*radius, cosf((GLfloat)(SPHERE_RESOLUTION/4-1)/(GLfloat)(SPHERE_RESOLUTION/4) * M_PI_2)*sinf(i/(GLfloat)(SPHERE_RESOLUTION/2) * M_PI)*radius);
    }
    glEnd();
}

void OpenGLSolids::DrawPointSphere(GLfloat radius)
{
    for(int i=-5; i<=5; i++)
    {
        glBegin(GL_POINTS);
        glNormal3f(0, -1.f, 0);
        glVertex3f(0, -radius, 0);
        glNormal3f(0, 1.f, 0);
        glVertex3f(0, radius, 0);
        
        for(int h=0; h<=24; h++)
        {
            glNormal3f(cosf(i/6.f*M_PI_2)*cosf(h/12.f*M_PI), sinf(i/6.f*M_PI_2), cosf(i/6.f*M_PI_2)*sinf(h/12.f*M_PI));
            glVertex3f(cosf(i/6.f*M_PI_2)*cosf(h/12.f*M_PI)*radius, sinf(i/6.f*M_PI_2)*radius, cosf(i/6.f*M_PI_2)*sinf(h/12.f*M_PI)*radius);
        }
        glEnd();
    }
}

void OpenGLSolids::DrawSolidCylinder(GLfloat radius, GLfloat height)
{
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<=24; i++)
    {
        glNormal3f(sinf(i/24.f*M_PI*2), 0.0, cosf(i/24.f*M_PI*2));
        glTexCoord2f(i/24.f, 0);
        glVertex3f(sinf(i/24.f*M_PI*2)*radius, height/2.0, cosf(i/24.f*M_PI*2)*radius);
        glTexCoord2f(i/24.f, height);
        glVertex3f(sinf(i/24.f*M_PI*2)*radius, -height/2.0, cosf(i/24.f*M_PI*2)*radius);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1.f, 0);
    glTexCoord2f(0.5, 0.5);
    glVertex3f(0, height/2.0, 0);
    for(int i=0; i<=24; i++)
    {
        glTexCoord2f(0.5+0.5*sinf(i/24.f*M_PI*2), 0.5+0.5*cosf(i/24.f*M_PI*2));
        glVertex3f(sinf(i/24.f*M_PI*2)*radius, height/2.0, cosf(i/24.f*M_PI*2)*radius);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1.f, 0);
    glTexCoord2f(0.5, 0.5);
    glVertex3f(0, -height/2.0, 0);
    for(int i=24; i>=0; i--)
    {
        glTexCoord2f(0.5+0.5*sinf(i/24.f*M_PI*2), 0.5+0.5*cosf(i/24.f*M_PI*2));
        glVertex3f(sinf(i/24.f*M_PI*2)*radius, -height/2.0, cosf(i/24.f*M_PI*2)*radius);
    }
    glEnd();
}

void OpenGLSolids::DrawSolidTorus(GLfloat majorRadius, GLfloat minorRadius)
{
    for(int i=0; i<48; i++)
    {
        GLfloat alpha0 = i/48.f*M_PI*2.f;
        GLfloat alpha1 = (i+1)/48.f*M_PI*2.f;
        
        glBegin(GL_TRIANGLE_STRIP);
        for(int h=0; h<=24; h++)
        {
            GLfloat ry = cosf(h/24.f*M_PI*2.f) * minorRadius;
            GLfloat rx = sinf(h/24.f*M_PI*2.f) * minorRadius;
            
            glNormal3f(sinf(h/24.f*M_PI*2.f)*cosf(alpha0), cosf(h/24.f*M_PI*2.f), sinf(h/24.f*M_PI*2.f)*sinf(alpha0));
            glVertex3f((rx + majorRadius)*cosf(alpha0), ry, (rx + majorRadius)*sinf(alpha0));
            glNormal3f(sinf(h/24.f*M_PI*2.f)*cosf(alpha1), cosf(h/24.f*M_PI*2.f), sinf(h/24.f*M_PI*2.f)*sinf(alpha1));
            glVertex3f((rx + majorRadius)*cosf(alpha1), ry, (rx + majorRadius)*sinf(alpha1));
        }
        glEnd();
    }
}