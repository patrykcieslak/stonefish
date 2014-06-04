//
//  Console.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Console__
#define __Stonefish_Console__

#include "common.h"
#include <SDL_thread.h>
#include "OpenGLPrinter.h"
#include "Sensor.h"

//font
#define FONT_NAME "/Library/Fonts/Arial.ttf"
#define FONT_SIZE 12
#define SCREEN_DPI 72

//shortcut
#define cInfo(format, ...)     Console::getInstance()->Print(0, format, ##__VA_ARGS__)
#define cWarning(format, ...)  Console::getInstance()->Print(1, format, ##__VA_ARGS__)
#define cError(format, ...)    Console::getInstance()->Print(2, format, ##__VA_ARGS__)

typedef struct
{
    int type;
    std::string text;
}
ConsoleMessage;

//singleton
class Console
{
public:
    void SetRenderSize(GLint w, GLint h);
    void Render();
    void Clear();
    void Print(int messageType, const char* format, ...);
    
    SDL_mutex* getLinesMutex();
    
    static Console* getInstance();
    
private:
    Console();
    ~Console();
    
    GLint windowW, windowH;
    std::vector<ConsoleMessage> lines;
    OpenGLPrinter* printer;
    GLuint logoTexture;
    SDL_mutex* linesMutex;
    
    static Console* instance;
};

#endif
