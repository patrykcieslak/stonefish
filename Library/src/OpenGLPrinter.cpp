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
    else
    {
        font->setCompileMode(OGLFT::Face::GlyphCompileMode::COMPILE);
        lastFontSize = size;
        lastFontColor[0] = lastFontColor[1] = lastFontColor[2] = 0.f;
        lastFontColor[3] = 1.f;
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
	glm::mat4 proj = glm::ortho(0.f, (GLfloat)windowW, 0.f, (GLfloat)windowH, -1.f, 1.f);
	glLoadMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void OpenGLPrinter::Print(GLfloat *color, GLfloat x, GLfloat y, GLfloat size, const char* text)//, ...)
{
    if(font != NULL)
    {
        if(size != lastFontSize)
        {
            font->setPointSize(size);
            lastFontSize = size;
        }
        if(lastFontColor[0] != color[0] || lastFontColor[1] != color[1] || lastFontColor[2] != color[2] || lastFontColor[3] != color[3])
        {
            font->setForegroundColor(color);
            lastFontColor[0] = color[0];
            lastFontColor[1] = color[1];
            lastFontColor[2] = color[2];
            lastFontColor[3] = color[3];
        }
        font->draw(x, y, text);
    }
}

GLfloat OpenGLPrinter::TextLength(const char *text)
{
    OGLFT::BBox bounds = font->measure(text);
    return bounds.x_max_ - bounds.x_min_;
}

glm::vec2 OpenGLPrinter::TextDimensions(const char *text)
{
    OGLFT::BBox bounds = font->measure(text);
    return glm::vec2(bounds.x_max_ - bounds.x_min_, bounds.y_max_ - bounds.y_min_);
}

void OpenGLPrinter::Finish()
{
    glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glPopAttrib();
}