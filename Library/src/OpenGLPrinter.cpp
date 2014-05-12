//
//  OpenGLPrinter.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPrinter.h"

#include <stdio.h>

GLint OpenGLPrinter::windowW = 800;
GLint OpenGLPrinter::windowH = 600;

void OpenGLPrinter::SetWindowSize(GLint width, GLint height)
{
    windowW = width;
    windowH = height;
}

OpenGLPrinter::OpenGLPrinter(const char* fontPath, GLint size, GLint dpi)
{
    font = new OGLFT::MonochromeTexture(fontPath, size, dpi);
    if(font == NULL || !font->isValid())
    {
        printf("Could not construct font!\n");
        font = NULL;
    }
}

OpenGLPrinter::~OpenGLPrinter()
{
    if(font != NULL)
        delete font;
}

void OpenGLPrinter::Start()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, windowW, 0.0, windowH, -100.0, 100.0);
    
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void OpenGLPrinter::Print(GLfloat *color, GLfloat x, GLfloat y, GLfloat size, const char* text)//, ...)
{
    if(font != NULL)
    {
        font->setForegroundColor(color); //invalidates cache
        font->setPointSize(size);        //invalidates cache
        font->draw(x, y, text);
    }
}

GLfloat OpenGLPrinter::TextLength(const char *text)
{
    OGLFT::BBox bounds = font->measure(text);
    return bounds.x_max_ - bounds.x_min_;
}

void OpenGLPrinter::Finish()
{
    glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glPopAttrib();
}