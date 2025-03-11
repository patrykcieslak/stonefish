/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

#define STANDARD_FONT_NAME "Lato-Medium.ttf"
#define STANDARD_FONT_SIZE 11
#define STANDARD_FONT_BASELINE 9

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
         \param fontPath a path to the font file
         \param size the native size of the font
         */
        OpenGLPrinter(const std::string& fontPath, GLuint size);
        
        //! A destructor.
        ~OpenGLPrinter();
        
        //! A method used to print text.
        /*!
         \param text a string to be printed
         \param color the desired color of the text
         \param x the x position of the text in window
         \param y the y position of the text in window
         \param size the height of the font
         \param raw flag indicating if raw OpenGL calls should be used
         */
        void Print(const std::string& text, glm::vec4 color, GLuint x, GLuint y, GLfloat size, bool raw = false);
        
        //! A method used to measure the length of a string in pixels.
        /*!
         \param text the string to be measured
         \return the length of the string in pixels
         */
        GLuint TextLength(const std::string& text);
        
        //! A method used to measure the sise of the string in pixels.
        /*!
         \param text the string to be measured
         \return a 2D vector containing width and height of the text in pixels
         */
        glm::ivec2 TextDimensions(const std::string& text);
        
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
