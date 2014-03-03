//
//  OpenGLPrinter.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPrinter.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "OpenGLUtil.h"

GLint OpenGLPrinter::windowW = 800;
GLint OpenGLPrinter::windowH = 600;

void OpenGLPrinter::SetWindowSize(GLint width, GLint height)
{
    windowW = width;
    windowH = height;
}

OpenGLPrinter::OpenGLPrinter(const char* fontPath, GLint size, GLfloat spacing)
{
	font = new FTTextureFont(fontPath);
    font->FaceSize(size, size);
    font->UseDisplayList(true);
    font->CharMap(ft_encoding_unicode);
    this->spacing = spacing;
}

OpenGLPrinter::~OpenGLPrinter()
{
    delete font;
}

void OpenGLPrinter::Start()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable (GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, windowW, windowH, 0.0, 0.0, 1.0);
    
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void OpenGLPrinter::Print(GLfloat *color, GLfloat x, GLfloat y, GLfloat size, const char* text)//, ...)
{
    float fontSize = (float)font->FaceSize();
    
    glPushMatrix();
    glTranslatef(x, y+fontSize/2.f, 0);
    glScalef(size/fontSize, -size/fontSize, size/fontSize);
    
    glColor4fv(color);
    font->Render(text, -1, FTPoint(), FTPoint(spacing, 0));
    
    glPopMatrix();
}

GLfloat OpenGLPrinter::TextLength(const char *text)
{
    return font->Advance(text);
}

void OpenGLPrinter::Finish()
{
    glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glPopAttrib();
}