//
//  Console.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Console__
#define __Stonefish_Console__

#include <SDL2/SDL_thread.h>
#include "StonefishCommon.h"
#include "graphics/OpenGLPrinter.h"

//shortcut
#define cInfo(format, ...)     Console::getInstance()->Print(0, format, ##__VA_ARGS__)
#define cWarning(format, ...)  Console::getInstance()->Print(1, format, ##__VA_ARGS__)
#define cError(format, ...)    Console::getInstance()->Print(2, format, ##__VA_ARGS__)
#define cCritical(format, ...) {Console::getInstance()->Print(3, format, ##__VA_ARGS__);abort();}

namespace sf
{

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
	void Init(GLuint w, GLuint h);
    void Render(bool overlay);
    void Clear();
    void Print(int messageType, const char* format, ...);
    void Scroll(GLfloat amount);
    void ResetScroll();
    
    SDL_mutex* getLinesMutex();
    
    static Console* getInstance();
    
private:
    Console();
    ~Console();
    
    GLuint windowW, windowH;
	GLuint consoleVAO;
	GLuint texQuadVBO;
	GLSLShader* texQuadShader;
    std::vector<ConsoleMessage> lines;
    OpenGLPrinter* printer;
    GLuint logoTexture;
    SDL_mutex* linesMutex;
    GLfloat scrollOffset;
    GLfloat scrollVelocity;
	bool ready;
	int64_t lastTime;
    
    static Console* instance;
};

}

#endif
