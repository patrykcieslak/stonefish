//
//  OpenGLPrinter.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/06/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPrinter__
#define __Stonefish_OpenGLPrinter__

#include "GLSLShader.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>

struct Character 
{
	GLuint texture;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
};

class OpenGLPrinter
{
public:
    OpenGLPrinter(const char* fontPath, GLuint size);
    ~OpenGLPrinter();

    void Print(glm::vec4 color, GLfloat x, GLfloat y, GLfloat size, const char* text);
    GLuint TextLength(const char* text);
    glm::ivec2 TextDimensions(const char* text);
    
    static void SetWindowSize(GLuint width, GLuint height);
    
private:
	bool initialized;
	GLuint fontVBO;
	GLuint nativeFontSize;
	std::map<GLchar, Character> chars;
	
	static GLSLShader* printShader;
	static GLuint windowW, windowH;
};


#endif
