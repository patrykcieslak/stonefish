//
//  Console.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Console.h"

Console* Console::instance = NULL;

Console::Console()
{
    windowW = 800;
    windowH = 600;
    printer = new OpenGLPrinter(FONT_NAME, FONT_SIZE, SCREEN_DPI);
}

Console::~Console()
{
    lines.clear();
    delete printer;
}

void Console::SetRenderSize(GLint w, GLint h)
{
    windowW = w;
    windowH = h;
}

void Console::Render()
{
    if(lines.size() == 0)
        return;
    
    //Set rendering params
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for OGLFT
    
    //Calculate visible lines range
    long int visibleLines = (windowH - 20) / 20;
    long int linesCount = lines.size();
    if(linesCount < visibleLines)
        visibleLines = linesCount;
    
    //Rendering
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLfloat)windowW, 0.0, (GLfloat)windowH, -100.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
    
    GLfloat info[4] = {1.0, 1.0, 1.0, 1.0};
    GLfloat warning[4] = {1.0, 1.0, 0.0, 1.0};
    GLfloat error[4] = {1.0, 0.0, 0.0, 1.0};
    GLfloat* colors[3] = {info, warning, error};
    
    for(long int i = 0; i < visibleLines; i++)
    {
        ConsoleMessage* msg = &lines[linesCount-1-i];
        printer->Print(colors[msg->type], 10.f, 10.f + i * 20.f, FONT_SIZE, msg->text.c_str());
    }
    
    glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glPopAttrib();
}

void Console::Print(int messageType, const char *format, ...)
{
    va_list args;
    char buffer[4096];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    ConsoleMessage msg;
    msg.type = messageType;
    msg.text = std::string(buffer);
    lines.push_back(msg);
}

void Console::Clear()
{
    lines.clear();
}

Console* Console::getInstance()
{
    if(instance == NULL)
        instance = new Console();
    
    return instance;
}