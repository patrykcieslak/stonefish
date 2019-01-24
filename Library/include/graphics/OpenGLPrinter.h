//
//  OpenGLPrinter.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/06/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPrinter__
#define __Stonefish_OpenGLPrinter__

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include "graphics/OpenGLDataStructs.h"

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
    //! A structure holding data of a single character.
    struct Character
    {
        glm::vec2 advance;
        glm::vec2 size;
        glm::vec2 bearing;
        GLfloat offset;
    };
    
    class GLSLShader;
    
    //! A class implementing an OpenGL text printer.
    class OpenGLPrinter
    {
    public:
        //! A constructor.
        /*!
         \param fontPath the path to the font file
         \param size the native size of the font
         */
        OpenGLPrinter(const char* fontPath, GLuint size);
        
        //! A destructor.
        ~OpenGLPrinter();
        
        //! A method used to print text.
        /*!
         \param text a pointer to the string to be printed
         \param color the desired color of the text
         \param x the x position of the text in window
         \param y the y position of the text in window
         \param size the size of the font
         */
        void Print(const char* text, glm::vec4 color, GLuint x, GLuint y, GLuint size);
        
        //! A method used to measure the length of a string in pixels.
        /*!
         \param text a pointer to the string
         \return the length of the string in pixels
         */
        GLuint TextLength(const char* text);
        
        //! A method used to measure the sise of the string in pixels.
        /*!
         \param text a pointer to the string
         \return a 2D vector containing width and height of the text in pixels
         */
        glm::ivec2 TextDimensions(const char* text);
        
        //! A static method used to set the widow size for the printers.
        /*!
         \param width the width of the window [pix]
         \param height the height of hte window [pix]
         */
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
