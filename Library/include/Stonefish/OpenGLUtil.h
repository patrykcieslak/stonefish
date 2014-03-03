//
//  OpenGLUtil.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLUtil__
#define __Stonefish_OpenGLUtil__

#include <stdint.h>
#include "common.h"

uint64_t GetTimeInMicroseconds();
void GetCWD(char* buffer,int length);
const char* GetDataPathPrefix(const char* directory);

GLuint LoadTexture(const char* filename);
GLuint LoadInternalTexture(const char* filename);
GLhandleARB LoadShader(GLenum shaderType, const char *filename, GLint *shaderCompiled);
GLhandleARB CreateProgramObject(GLhandleARB vertexShader, GLhandleARB fragmentShader);
void LinkProgram(GLhandleARB programObject, GLint *programLinked);
GLboolean CheckForExtension(const GLchar *extensionName, const GLubyte *extensions);
GLboolean CheckShadersAvailable();
void SetFloatvFromMat(const btMatrix3x3 &mat, GLfloat* fv);

#endif 

