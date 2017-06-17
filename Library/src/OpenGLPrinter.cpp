//
//  OpenGLPrinter.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/06/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLPrinter.h"
#include <stdio.h>

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
			FT_Set_Pixel_Sizes(face, 0, nativeFontSize);
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			
			//Load all characters
			for(GLubyte c = 0; c < 128; ++c)
			{
				//Load character glyph 
				if(FT_Load_Char(face, c, FT_LOAD_RENDER))
				{
					printf("Freetype: Failed to load Glyph '%c'!", c);
					continue;
				}
				
				//Generate texture
				GLuint texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				
				Character character = {texture,
									   glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
									   glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
									   (GLuint)face->glyph->advance.x};
				
				chars.insert(std::pair<GLchar, Character>(c, character));
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			//Freetype not needed any more
			FT_Done_Face(face);
			FT_Done_FreeType(ft);

			glGenBuffers(1, &fontVBO); //Generate VBO for rendering textured quads
			glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			
			//Successfully initialized!
			initialized = true;
		}
	}
}

OpenGLPrinter::~OpenGLPrinter()
{
	//Destroy all textures
	for(auto it = chars.begin(); it != chars.end(); ++it) 
		glDeleteTextures(1, &it->second.texture);
		
	if(fontVBO != 0)
		glDeleteBuffers(1,&fontVBO);
}

void OpenGLPrinter::Print(glm::vec4 color, GLfloat x, GLfloat y, GLfloat size, const char* text)
{
	if(!initialized)
		return;
	
	GLfloat sx = 2.f/(GLfloat)windowW;
	GLfloat sy = 2.f/(GLfloat)windowH;
	GLfloat scale = size/nativeFontSize;
	
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	
	printShader->Enable();
	printShader->SetUniform("color", color);
	printShader->SetUniform("tex", 1);

	for(const char *c = text; *c; ++c) 
	{
		Character ch = chars[*c];
		
		float x2 = (x + ch.bearing.x) * sx - 1.0f;
		float y2 = (-y - ch.bearing.y) * sy + 1.0f;
		float w = ch.size.x * sx * scale;
		float h = ch.size.y * sy * scale;
		
		GLfloat box[4][4] = {{x2,     -y2    , 0, 0},
							 {x2 + w, -y2    , 1, 0},
							 {x2,     -y2 - h, 0, 1},
							 {x2 + w, -y2 - h, 1, 1}};
  
		glBindTexture(GL_TEXTURE_2D, ch.texture);		
		glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(box), box);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		x += (ch.advance >> 6) * scale;
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	printShader->Disable();
}

GLuint OpenGLPrinter::TextLength(const char *text)
{
	GLuint length = 0;
	for(const char *c = text; *c; ++c) 
		length += chars[*c].advance >> 6;
    return length;
}

glm::ivec2 OpenGLPrinter::TextDimensions(const char *text)
{
    return glm::ivec2(TextLength(text), nativeFontSize);
}