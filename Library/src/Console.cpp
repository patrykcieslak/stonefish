//
//  Console.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Console.h"
#include "SystemUtil.h"
#include "stb_image.h"
#include "IMGUI.h"

Console* Console::instance = NULL;

Console::Console()
{
    windowW = 800;
    windowH = 600;
    printer = new OpenGLPrinter(FONT_NAME, FONT_SIZE, SCREEN_DPI);
    scrollOffset = 0;
    scrollVelocity = 0;
    
    //Load logo texture - can't use material class because it writes to the console
    char path[1024];
    int width, height, channels;
    GetDataPath(path, 1024-32);
    strcat(path, "logo_color.png");
    
    // Allocate image; fail out on error
    unsigned char* dataBuffer = stbi_load(path, &width, &height, &channels, 4);
    if(dataBuffer != NULL)
    {
        // Allocate an OpenGL texture
        glGenTextures(1, &logoTexture);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        // Upload texture to memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
        // Set certain properties of texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Wrap texture around
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Release internal buffer
        stbi_image_free(dataBuffer);
    }
    else
        logoTexture = 0;
    
    linesMutex = SDL_CreateMutex();
}

Console::~Console()
{
    lines.clear();
    delete printer;
    if(logoTexture > 0)
        glDeleteTextures(1, &logoTexture);
    SDL_DestroyMutex(linesMutex);
}

void Console::SetRenderSize(GLint w, GLint h)
{
    windowW = w;
    windowH = h;
}

void Console::Scroll(GLfloat amount)
{
    scrollOffset += 1.f * amount;
    scrollVelocity = 25.f * amount;
}

void Console::ResetScroll()
{
    scrollOffset = 0.f;
    scrollVelocity = 0.f;
}

void Console::Render(bool overlay, GLfloat dt)
{
    if(lines.size() == 0)
        return;
    
    //Calculate visible lines range
    long int maxVisibleLines = (long int)floorf((GLfloat)windowH/(GLfloat)(FONT_SIZE + 5)) + 1;
    long int linesCount = lines.size();
    long int visibleLines = maxVisibleLines;
    long int scrolledLines = 0;
    
    if(linesCount < maxVisibleLines) //there is enough space to display all lines
    {
        scrollOffset = 0.f;
        scrollVelocity = 0.f;
        visibleLines = linesCount;
    }
    else //there is more lines than can appear on screen at once
    {
        scrolledLines = (long int)floorf(-scrollOffset /(GLfloat)(FONT_SIZE + 5));
        scrolledLines = scrolledLines < 0 ? 0 : scrolledLines;
        if(linesCount < scrolledLines + visibleLines)
            visibleLines = linesCount - scrolledLines;

        if(scrollVelocity != 0.f) //velocity damping (momentum effect)
        {
            scrollOffset += scrollVelocity * dt;
            GLfloat dampingAcc = -scrollVelocity * 3.f;
            scrollVelocity += dampingAcc * dt;
        }
        
        //springy effect
        if(scrollOffset > 0.f) //the list is scrolled up too much
            scrollVelocity -= scrollOffset * 5.f * dt;
        else if(visibleLines < (maxVisibleLines - 1)) //the list is scrolled down too much
            scrollVelocity += (1.f - (GLfloat)visibleLines/(GLfloat)(maxVisibleLines - 1)) * (GLfloat)windowH * 5.f * dt;
    }
    
    //Set rendering params
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    //Setup viewport and ortho
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glm::mat4 proj = glm::ortho(0.f, (GLfloat)windowW, 0.f, (GLfloat)windowH, -1.f, 1.f);
	glLoadMatrixf(glm::value_ptr(proj));
    
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
    
    if(overlay)
    {
        //Draw background
        glBindTexture(GL_TEXTURE_2D, IMGUI::getInstance()->getTranslucentTexture());
        glColor4f(0.4f, 0.4f, 0.4f, 1.f);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);
        glTexCoord2f(1.f, 0);
        glVertex2f(windowW, 0);
        glTexCoord2f(0, 1.f);
        glVertex2f(0, windowH);
        glTexCoord2f(1.f, 1.f);
        glVertex2f(windowW, windowH);
        glEnd();
    }
    
    GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
    
    glBindTexture(GL_TEXTURE_2D, logoTexture);
    glColor4f(1.f, 1.f, 1.f, 1.f);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);
    glVertex2f(windowW - logoSize - logoMargin, windowH - logoMargin);
    glTexCoord2f(0, 1.f);
    glVertex2f(windowW - logoSize - logoMargin, windowH - logoMargin - logoSize);
    glTexCoord2f(1.f, 0);
    glVertex2f(windowW - logoMargin, windowH - logoMargin);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(windowW - logoMargin, windowH - logoMargin - logoSize);
    glEnd();
    
    //Rendering params for text
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //for OGLFT
    
    //Rendering
    GLfloat info[4] = {1.f, 1.f, 1.f, 0.95f};
    GLfloat warning[4] = {1.f, 1.f, 0.f, 1.f};
    GLfloat error[4] = {1.f, 0.f, 0.f, 1.f};
    GLfloat* colors[3] = {info, warning, error};
    
    for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
    {
        ConsoleMessage* msg = &lines[linesCount-1-i];
        printer->Print(colors[msg->type], 10.f, scrollOffset + 10.f + i * (FONT_SIZE + 5), FONT_SIZE, msg->text.c_str());
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
    SDL_LockMutex(linesMutex);
    lines.push_back(msg);
    SDL_UnlockMutex(linesMutex);
}

void Console::Clear()
{
    SDL_LockMutex(linesMutex);
    lines.clear();
    SDL_UnlockMutex(linesMutex);
}

SDL_mutex* Console::getLinesMutex()
{
    return linesMutex;
}

Console* Console::getInstance()
{
    if(instance == NULL)
        instance = new Console();
    
    return instance;
}