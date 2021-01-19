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
//  OpenGLPrinter.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/06/17.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLPrinter.h"

#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include <stdio.h>
#ifdef EMBEDDED_RESOURCES
#include "ResourceHandle.h"
#endif

namespace sf
{

GLuint OpenGLPrinter::windowW = 800;
GLuint OpenGLPrinter::windowH = 600;
GLSLShader* OpenGLPrinter::printShader = NULL;

void OpenGLPrinter::SetWindowSize(GLuint width, GLuint height)
{
    windowW = width;
    windowH = height;
}

OpenGLPrinter::OpenGLPrinter(const std::string& fontPath, GLuint size)
{
    initialized = false;
    fontVBO = 0;
    nativeFontSize = size;
    
    FT_Error error = 0;
    FT_Library ft;
    FT_Face face;
    
    //Load Freetype library and font
    if((error = FT_Init_FreeType(&ft))) 
        printf("Freetype: Could not init library!\n");
    else
    {
#ifdef EMBEDDED_RESOURCES
        ResourceHandle rh(fontPath);
        error = FT_New_Memory_Face(ft, rh.data(), rh.size(), 0, &face);
#else
        error = FT_New_Face(ft, fontPath.c_str(), 0, &face);
#endif
        if(error)
        {
            printf("Freetype: Could not load font from: %s!\n", fontPath.c_str());
            FT_Done_FreeType(ft);
        }
    }
    
    if(!error)
    {
        //Load shader if not already loaded
        if(printShader == NULL)
        {
            printShader = new GLSLShader("printer.frag","printer.vert");
            printShader->AddUniform("tex", ParameterType::INT);
            printShader->AddUniform("color", ParameterType::VEC4);
        }
                
        if(printShader->isValid())
        {
            //Calculate texture atlas dimensions
            FT_Set_Pixel_Sizes(face, 0, nativeFontSize);
            
            unsigned int w = 0;
            unsigned int h = 0;
            
            for(int i = 32; i < 128; ++i)
            {
                if(FT_Load_Char(face, i, FT_LOAD_RENDER))
                {
                    fprintf(stderr, "Loading character %c failed!\n", i);
                    continue;
                }
                
                w += face->glyph->bitmap.width;
                h = face->glyph->bitmap.rows > h ? face->glyph->bitmap.rows : h;
            }
            
            texWidth = (GLfloat)w;
            texHeight = (GLfloat)h;
            
            //Create texture
            glGenTextures(1, &fontTexture);
            OpenGLState::BindTexture(TEX_GUI1, GL_TEXTURE_2D, fontTexture);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLint)w, (GLint)h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            //Load all characters
            GLuint x = 0;
            
            for(int i = 32; i < 128; ++i)
            {
                if(FT_Load_Char(face, i, FT_LOAD_RENDER))
                    continue;
                
                glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
                
                chars[i-32] = {glm::vec2((GLfloat)(face->glyph->advance.x >> 6), (GLfloat)(face->glyph->advance.y >> 6)),
                            glm::vec2((GLfloat)face->glyph->bitmap.width, (GLfloat)face->glyph->bitmap.rows),
                            glm::vec2((GLfloat)face->glyph->bitmap_left, (GLfloat)face->glyph->bitmap_top),
                            (GLfloat)x/texWidth};
                
                x += face->glyph->bitmap.width;
            }
            
            OpenGLState::UnbindTexture(TEX_GUI1);
            
            //Freetype not needed any more
            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            glGenBuffers(1, &fontVBO); //Generate VBO for rendering textured quads
            
            //Successfully initialized!
            initialized = true;
        }
    }
}

OpenGLPrinter::~OpenGLPrinter()
{
    //Destroy all textures
    if(fontTexture != 0)
        glDeleteTextures(1, &fontTexture);
        
    if(fontVBO != 0)
        glDeleteBuffers(1, &fontVBO);
}

void OpenGLPrinter::Print(const std::string& text, glm::vec4 color, GLuint x, GLuint y, GLfloat size, bool raw)
{
    if(!initialized)
        return;
    
    struct Point
    {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
    } coords[6 * text.length()];
    
    memset(coords, 0, sizeof coords);
    
    unsigned int n = 0;
    
    GLfloat scale = (GLfloat)size/(GLfloat)nativeFontSize;
    GLfloat xf = (GLfloat)x/(GLfloat)windowW * 2.f - 1.f;
    GLfloat yf = (GLfloat)y/(GLfloat)windowH * 2.f - 1.f;
    GLfloat sx = 2.f/(GLfloat)windowW;
    GLfloat sy = 2.f/(GLfloat)windowH;
    
    if(raw)
    {
        glActiveTexture(GL_TEXTURE0 + TEX_GUI1);
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glUseProgram(printShader->getProgramHandle());
    }
    else
    {
        OpenGLState::BindTexture(TEX_GUI1, GL_TEXTURE_2D, fontTexture);
        printShader->Use();
    }
    
    printShader->SetUniform("color", color);
    printShader->SetUniform("tex", TEX_GUI1);

    const char* ctext = text.c_str();
    for(const char *c = ctext; *c; ++c)
    {
        Character ch = chars[*c-32];
        GLfloat x2 = xf + ch.bearing.x * scale * sx;
        GLfloat y2 = -yf - ch.bearing.y * scale * sy;
        GLfloat w = ch.size.x * scale * sx;
        GLfloat h = ch.size.y * scale * sy;
        xf += ch.advance.x * scale * sx;
        yf += ch.advance.y * scale * sy;
        
        if(!w || !h)
            continue;
        
        coords[n++] = (Point){x2, -y2,     ch.offset, 0};
        coords[n++] = (Point){x2+w, -y2,   ch.offset + ch.size.x/texWidth, 0};
        coords[n++] = (Point){x2, -y2-h,   ch.offset, ch.size.y/texHeight};
        coords[n++] = (Point){x2+w, -y2,   ch.offset + ch.size.x/texWidth, 0};
        coords[n++] = (Point){x2, -y2-h,   ch.offset, ch.size.y/texHeight};
        coords[n++] = (Point){x2+w, -y2-h, ch.offset + ch.size.x/texWidth, ch.size.y/texHeight};
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof coords, coords, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
 
    glDrawArrays(GL_TRIANGLES, 0, n);
 
    if(raw)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }
    else
    {
        OpenGLState::UnbindTexture(TEX_GUI1);
        OpenGLState::UseProgram(0);
    }
}

GLuint OpenGLPrinter::TextLength(const std::string& text)
{
    GLuint length = 0;
    const char* ctext = text.c_str();
    for(const char *c = ctext; *c; ++c)
        length += chars[*c-32].advance.x + chars[*c-32].bearing.x;
    return length;
}

glm::ivec2 OpenGLPrinter::TextDimensions(const std::string& text)
{
    return glm::ivec2(TextLength(text), nativeFontSize);
}

}
