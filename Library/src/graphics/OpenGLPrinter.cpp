//
//  OpenGLPrinter.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/06/17.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLPrinter.h"

#include <stdio.h>

using namespace sf;

GLuint OpenGLPrinter::windowW = 800;
GLuint OpenGLPrinter::windowH = 600;
GLSLShader* OpenGLPrinter::printShader = NULL;

void OpenGLPrinter::SetWindowSize(GLuint width, GLuint height)
{
    windowW = width;
    windowH = height;
}

OpenGLPrinter::OpenGLPrinter(const char* fontPath, GLuint size)
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
		if((error = FT_New_Face(ft, fontPath, 0, &face))) 
		{
			printf("Freetype: Could not open font from file: %s!\n", fontPath);
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
			
            GLint w = 0;
            GLint h = 0;
            
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
            glActiveTexture(GL_TEXTURE0 + TEX_GUI1);
            glGenTextures(1, &fontTexture);
            glBindTexture(GL_TEXTURE_2D, fontTexture);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
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
            
            glBindTexture(GL_TEXTURE_2D, 0);
			
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

void OpenGLPrinter::Print(const char* text, glm::vec4 color, GLuint x, GLuint y, GLuint size)
{
	if(!initialized)
		return;
    
    struct Point
    {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
    } coords[6 * strlen(text)];
    
    memset(coords, 0, sizeof coords);
    
    unsigned int n = 0;
    
	GLfloat scale = (GLfloat)size/(GLfloat)nativeFontSize;
    GLfloat xf = (GLfloat)x/(GLfloat)windowW * 2.f - 1.f;
    GLfloat yf = (GLfloat)y/(GLfloat)windowH * 2.f - 1.f;
    GLfloat sx = 2.f/(GLfloat)windowW;
    GLfloat sy = 2.f/(GLfloat)windowH;
    
    glActiveTexture(GL_TEXTURE0 + TEX_GUI1);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
	printShader->Use();
	printShader->SetUniform("color", color);
	printShader->SetUniform("tex", TEX_GUI1);

	for(const char *c = text; *c; ++c) 
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
 
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

GLuint OpenGLPrinter::TextLength(const char *text)
{
	GLuint length = 0;
	for(const char *c = text; *c; ++c) 
        length += chars[*c].advance.x;
    return length;
}

glm::ivec2 OpenGLPrinter::TextDimensions(const char *text)
{
    return glm::ivec2(TextLength(text), nativeFontSize);
}
