//
//  OpenGLDebugDrawer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "OpenGLDebugDrawer.h"
#include "Console.h"

#pragma mark Constructors
OpenGLDebugDrawer::OpenGLDebugDrawer(int debugMode)
{
    setDebugMode(debugMode);
}

#pragma mark - Accessors
void OpenGLDebugDrawer::setDebugMode(int debugMode)
{
    mode = debugMode;
}

int OpenGLDebugDrawer::getDebugMode() const
{
    return mode;
}

#pragma mark - Methods
void OpenGLDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    glColor4f((GLfloat)color[0], (GLfloat)color[1], (GLfloat)color[2], 1.f);
    glBegin(GL_LINES);
    glBulletVertex(from);
    glBulletVertex(to);
    glEnd();
}

void OpenGLDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor)
{
    glBegin(GL_LINES);
    glColor4f((GLfloat)fromColor[0], (GLfloat)fromColor[1], (GLfloat)fromColor[2], 1.f);
    glBulletVertex(from);
    glColor4f((GLfloat)toColor[0], (GLfloat)toColor[1], (GLfloat)toColor[2], 1.f);
    glBulletVertex(to);
    glEnd();
}

void OpenGLDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
}

void OpenGLDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
}

void OpenGLDebugDrawer::reportErrorWarning(const char* warningString)
{
    cWarning(warningString);
}


