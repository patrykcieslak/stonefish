//
//  OpenGLPrinter.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/06/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPrinter__
#define __Stonefish_OpenGLPrinter__

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include "graphics/GLSLShader.h"

//Standard font
#ifdef __linux__
#define FONT_NAME "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-R.ttf"
#define FONT_SIZE 12
#define FONT_BASELINE 9
#elif __APPLE__
#define FONT_NAME "/Library/Fonts/Andale Mono.ttf"
#define FONT_SIZE 12
#define FONT_BASELINE 9
#endif

namespace sf
{
    //!
    struct Character
    {
        glm::vec2 advance;
        glm::vec2 size;
        glm::vec2 bearing;
        GLfloat offset;
    };
    
    //!
    class OpenGLPrinter
    {
    public:
        OpenGLPrinter(const char* fontPath, GLuint size);
        ~OpenGLPrinter();
        
        void Print(const char* text, glm::vec4 color, GLuint x, GLuint y, GLuint size);
        GLuint TextLength(const char* text);
        glm::ivec2 TextDimensions(const char* text);
        
        static void SetWindowSize(GLuint width, GLuint height);
        
    private:
        bool initialized;
        GLuint fontVBO;
        GLuint nativeFontSize;
        GLuint fontTexture;
        GLfloat texWidth;
        GLfloat texHeight;
        Character chars[128-32];
        
        static GLSLShader* printShader;
        static GLuint windowW, windowH;
    };
}

#endif
