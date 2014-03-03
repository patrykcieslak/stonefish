//
//  OpenGLUtil.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __MACH__
#include <mach/mach_time.h>
#include <mach/mach.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "stb_image.h"
#include <Carbon/Carbon.h>
#include "SimulationApp.h"

//Precise time
uint64_t GetTimeInMicroseconds()
{
#ifdef __MACH__
    return mach_absolute_time()/1000;
#else
    unsigned ticks;
	LARGE_INTEGER count;
	static bool firstTime = true;
	static bool haveTimer = false;
	static LARGE_INTEGER frequency;
	static LARGE_INTEGER baseCount;
    
	if (firstTime) {
		firstTime = false;
		haveTimer = QueryPerformanceFrequency(&frequency) ? true : false;
		QueryPerformanceCounter (&baseCount);
	}
    
	QueryPerformanceCounter (&count);
	count.QuadPart -= baseCount.QuadPart;
	ticks = (uint64_t)(count.QuadPart * LONGLONG (1000000) / frequency.QuadPart);
	return ticks;
#endif
}

void GetCWD(char* buffer, int length)
{
#ifdef _MSC_VER
	GetCurrentDirectory(length, buffer);
#else
    getcwd(buffer, length);
#endif
}

const char* GetDataPathPrefix(const char* directory)
{
    static char dataPathPrefix[PATH_MAX];
    
#ifdef __APPLE__
    
    CFStringRef dir = CFStringCreateWithCString(CFAllocatorGetDefault(), directory, kCFStringEncodingMacRoman);
    
    CFURLRef datafilesURL = CFBundleCopyResourceURL(CFBundleGetMainBundle(), dir, 0, 0);
    
    CFURLGetFileSystemRepresentation(datafilesURL, true, reinterpret_cast<UInt8*>(dataPathPrefix), PATH_MAX);
    
    if(datafilesURL != NULL)
        CFRelease(datafilesURL);
    
    CFRelease(dir);
#else
    char* envDataPath = 0;
    
    // get data path from environment var
    envDataPath = getenv(DATAPATH_VAR_NAME);
    
    // set data path prefix / base directory.  This will
    // be either from an environment variable, or from
    // a compiled in default based on original configure
    // options
    if (envDataPath != 0)
        strcpy(dataPathPrefix, envDataPath);
    else
        strcpy(dataPathPrefix, CEGUI_SAMPLE_DATAPATH);
#endif
    
    return dataPathPrefix;
}

//Textures
GLuint LoadTexture(const char* filename)
{
    int width, height, channels;
    GLuint texture;
    
    // Allocate image; fail out on error
    unsigned char* dataBuffer = stbi_load(filename, &width, &height, &channels, 4);
    if(dataBuffer == NULL)
        return -1;
    
    // Allocate an OpenGL texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
    
    // Release internal buffer
    stbi_image_free(dataBuffer);
    
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    
    // Set certain properties of texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
    // Wrap texture around
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Done setting image parameters
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

GLuint LoadInternalTexture(const char* filename)
{
    char path[1024];
    GetCWD(path, 1024);
    strcat(path, "/");
    strcat(path, SimulationApp::getApp()->getDataPath());
    strcat(path, "/");
    strcat(path, filename);
    
    return LoadTexture(path);
}

//Extensions
GLboolean CheckForExtension(const GLchar *extensionName, const GLubyte *extensions)
{
	GLboolean extensionAvailable;
#ifdef _MSC_VER
	extensionAvailable = glewIsSupported(extensionName);
#else
	extensionAvailable = gluCheckExtension((GLubyte *)extensionName, extensions);
#endif
	return extensionAvailable;
}

//Shaders
GLboolean CheckShadersAvailable()
{
    GLboolean exts[4];
#ifdef _MSC_VER
	exts[0] = glewIsSupported("GL_ARB_shader_objects");
	exts[1] = glewIsSupported("GL_ARB_shading_language_100");
	exts[2] = glewIsSupported("GL_ARB_vertex_shader");
	exts[3] = glewIsSupported("GL_ARB_fragment_shader");
#else
    const GLubyte *extensions = glGetString(GL_EXTENSIONS);
    exts[0] = gluCheckExtension((GLubyte*)"GL_ARB_shader_objects", extensions);
    exts[1] = gluCheckExtension((GLubyte*)"GL_ARB_shading_language_100", extensions);
    exts[2] = gluCheckExtension((GLubyte*)"GL_ARB_vertex_shader", extensions);
    exts[3] = gluCheckExtension((GLubyte*)"GL_ARB_fragment_shader", extensions);
#endif
    return exts[0] && exts[1] && exts[2] && exts[3];
}

//#endif

GLhandleARB LoadShader(GLenum shaderType, const char *filename, GLint *shaderCompiled)
{
	GLhandleARB shaderObject = NULL;

    char path[1024];
    GetCWD(path, 1024);
    strcat(path, "/");
    strcat(path, SimulationApp::getApp()->getShaderPath());
    strcat(path, "/");
    strcat(path, filename);

    FILE *fp = fopen(path, "rb");
	if(fp == NULL)
		return 0;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* shaderSource = new char[size+1];
	long readsize = fread(shaderSource, sizeof(char), size, fp);
    if(readsize != size)
		return 0;

    shaderSource[size] = '\0';
	fclose(fp);

	if(shaderSource!= NULL)
	{
		GLint infoLogLength = 0;
		shaderObject = glCreateShaderObjectARB(shaderType);
		
		glShaderSourceARB(shaderObject, 1, (const GLcharARB**)&shaderSource, NULL);
		glCompileShaderARB(shaderObject);
		
		glGetObjectParameterivARB(shaderObject, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
		
		if( infoLogLength > 0 )
		{
			GLcharARB *infoLog = (GLcharARB *)malloc(infoLogLength);
			
			if( infoLog != NULL )
			{
				glGetInfoLogARB(shaderObject,
								infoLogLength,
								&infoLogLength,
								infoLog);
				
				printf(">> Shader compile log:\n%s\n", infoLog);
				free(infoLog);
			}
		}
		
		glGetObjectParameterivARB(shaderObject, GL_OBJECT_COMPILE_STATUS_ARB, shaderCompiled);
		
		if(*shaderCompiled == 0)
			printf(">> Failed to compile shader: %s", shaderSource);
        
        free(shaderSource);
	}
	else
		*shaderCompiled = 1;

	return shaderObject;
}

void LinkProgram(GLhandleARB programObject, GLint *programLinked)
{
	GLint infoLogLength = 0;
	
	glLinkProgramARB(programObject);
	
	glGetObjectParameterivARB(programObject, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
	
	if(infoLogLength > 0)
	{
		GLcharARB *infoLog = (GLcharARB *)malloc(infoLogLength);
		
		if( infoLog != NULL)
		{
			glGetInfoLogARB(programObject,
							infoLogLength,
							&infoLogLength,
							infoLog);
			
			printf(">> Program link log:\n%s\n", infoLog);
			free(infoLog);
		}
	}
	
	glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, programLinked);
	
	if(*programLinked == 0)
		printf(">> Failed to link program %s\n", (GLubyte *)&programObject);
}

GLhandleARB CreateProgramObject(GLhandleARB vertexShader, GLhandleARB fragmentShader)
{
	GLint programLinked = 0;
	
	GLhandleARB programObject = glCreateProgramObjectARB();
	
	glAttachObjectARB(programObject, vertexShader);
	glDeleteObjectARB(vertexShader);
	
	glAttachObjectARB(programObject, fragmentShader);
	glDeleteObjectARB(fragmentShader);
	
	LinkProgram(programObject, &programLinked);
	
	if(!programLinked)
	{
		glDeleteObjectARB(programObject);
		programObject = NULL;
	}
	
	return programObject;
}

void SetFloatvFromMat(const btMatrix3x3 &mat, GLfloat* fv)
{
    fv[0] = btScalar(mat.getRow(0).x());
    fv[1] = btScalar(mat.getRow(1).x());
    fv[2] = btScalar(mat.getRow(2).x());
    fv[3] = btScalar(mat.getRow(0).y());
    fv[4] = btScalar(mat.getRow(1).y());
    fv[5] = btScalar(mat.getRow(2).y());
    fv[6] = btScalar(mat.getRow(0).z());
    fv[7] = btScalar(mat.getRow(1).z());
    fv[8] = btScalar(mat.getRow(2).z());
}
