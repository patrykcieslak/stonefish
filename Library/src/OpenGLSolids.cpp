//
//  OpenGLSolids.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSolids.h"
#include <math.h>

void SetupOrtho()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void DrawScreenAlignedQuad()
{
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1, 0);
    glVertex3f(1.f, -1.0f, 0.0f);
    glTexCoord2f(1, 1);
    glVertex3f(1.f, 1.f, 0.0f);
    glTexCoord2f(0, 1);
    glVertex3f(-1.0f, 1.f, 0.0f);
    glEnd();
}

void DrawCoordSystem(GLfloat size)
{
    glBegin(GL_LINES);
    glColor4f(1.f, 0.f, 0.f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(size, 0, 0);
    
    glColor4f(0.f, 1.f, 0.f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, size, 0);
    
    glColor4f(0.f, 0.f, 1.f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, size);
    glEnd();
}

void DrawPoint(GLfloat size)
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

void DrawSolidBox(GLfloat halfX, GLfloat halfY, GLfloat halfZ)
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

void DrawSolidSphere(GLfloat radius)
{
    for(int i=-5; i<5; i++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for(int h=0; h<=24; h++)
        {
            glNormal3f(cosf(i/6.f*M_PI_2)*cosf(h/12.f*M_PI), sinf(i/6.f*M_PI_2), cosf(i/6.f*M_PI_2)*sinf(h/12.f*M_PI));
            glVertex3f(cosf(i/6.f*M_PI_2)*cosf(h/12.f*M_PI)*radius, sinf(i/6.f*M_PI_2)*radius, cosf(i/6.f*M_PI_2)*sinf(h/12.f*M_PI)*radius);
            
            glNormal3f(cosf((i+1)/6.f*M_PI_2)*cosf(h/12.f*M_PI), sinf((i+1)/6.f*M_PI_2), cosf((i+1)/6.f*M_PI_2)*sinf(h/12.f*M_PI));
            glVertex3f(cosf((i+1)/6.f*M_PI_2)*cosf(h/12.f*M_PI)*radius, sinf((i+1)/6.f*M_PI_2)*radius, cosf((i+1)/6.f*M_PI_2)*sinf(h/12.f*M_PI)*radius);
        }
        glEnd();
    }
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1.f, 0);
    glVertex3f(0, -radius, 0);
    for(int i=0; i<=24; i++)
    {
        glNormal3f(cosf(-5.f/6.f*M_PI_2)*cosf(i/12.f*M_PI), sinf(-5.f/6.f*M_PI_2), cosf(-5.f/6.f*M_PI_2)*sinf(i/12.f*M_PI));
        glVertex3f(cosf(-5.f/6.f*M_PI_2)*cosf(i/12.f*M_PI)*radius, sinf(-5.f/6.f*M_PI_2)*radius, cosf(-5.f/6.f*M_PI_2)*sinf(i/12.f*M_PI)*radius);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1.f, 0);
    glVertex3f(0, radius, 0);
    for(int i=24; i>=0; i--)
    {
        glNormal3f(cosf(5.f/6.f*M_PI_2)*cosf(i/12.f*M_PI), sinf(5.f/6.f*M_PI_2), cosf(5.f/6.f*M_PI_2)*sinf(i/12.f*M_PI));
        glVertex3f(cosf(5.f/6.f*M_PI_2)*cosf(i/12.f*M_PI)*radius, sinf(5.f/6.f*M_PI_2)*radius, cosf(5.f/6.f*M_PI_2)*sinf(i/12.f*M_PI)*radius);
    }
    glEnd();
}

void DrawPointSphere(GLfloat radius)
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

void DrawSolidCylinder(GLfloat radius, GLfloat height)
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