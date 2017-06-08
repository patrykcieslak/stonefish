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