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

Console* Console::instance = NULL;

Console::Console()
{
    windowW = 800;
    windowH = 600;
    printer = new OpenGLPrinter(FONT_NAME, FONT_SIZE, SCREEN_DPI);
    
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    //Setup viewport and ortho
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLfloat)windowW, 0.0, (GLfloat)windowH, -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
    
    //Draw stonefish logo/name/version/author
    /*GLfloat logoSize = 256.f;
    
    glBindTexture(GL_TEXTURE_2D, logoTexture);
    glColor4f(1.f, 1.f, 1.f, 0.3f);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);
    glVertex2f(windowW/2.f - logoSize/2.f, windowH/2.f + logoSize/2.f);
    glTexCoord2f(0, 1.f);
    glVertex2f(windowW/2.f - logoSize/2.f, windowH/2.f - logoSize/2.f);
    glTexCoord2f(1.f, 0);
    glVertex2f(windowW/2.f + logoSize/2.f, windowH/2.f + logoSize/2.f);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(windowW/2.f + logoSize/2.f, windowH/2.f - logoSize/2.f);
    glEnd();*/
    
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
    
    //Calculate visible lines range
    long int visibleLines = (long int)floorf((GLfloat)(windowH - 20)/(GLfloat)(FONT_SIZE * 1.3f));
    long int linesCount = lines.size();
    if(linesCount < visibleLines)
        visibleLines = linesCount;
    
    //Rendering
    GLfloat info[4] = {1.0, 1.0, 1.0, 1.0};
    GLfloat warning[4] = {1.0, 1.0, 0.0, 1.0};
    GLfloat error[4] = {1.0, 0.0, 0.0, 1.0};
    GLfloat* colors[3] = {info, warning, error};
    
    for(long int i = 0; i < visibleLines; i++)
    {
        ConsoleMessage* msg = &lines[linesCount-1-i];
        printer->Print(colors[msg->type], 10.f, 10.f + i * FONT_SIZE * 1.3f, FONT_SIZE, msg->text.c_str());
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