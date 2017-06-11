//
//  GLSLShader.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "GLSLShader.h"
#include "SystemUtil.hpp"
#include "Console.h"

GLhandleARB GLSLShader::saqVertexShader = NULL;

GLSLShader::GLSLShader(const char* fragment, const char* vertex)
{
    valid = false;
    enabled = false;
    GLint compiled = 0;
    GLhandleARB vs;
    GLhandleARB fs;
    
    if(vertex == NULL)
        vs = saqVertexShader;
    else
        vs = LoadShader(GL_VERTEX_SHADER, vertex, &compiled);
    
    fs = LoadShader(GL_FRAGMENT_SHADER, fragment, &compiled);
    
    shader = CreateProgramObject(vs, fs);
    LinkProgram(shader, &compiled);
    
    valid = true;
}

GLSLShader::~GLSLShader()
{
    if(valid)
        glDeleteObjectARB(shader);
}

bool GLSLShader::isValid()
{
    return valid;
}

bool GLSLShader::isEnabled()
{
    return enabled;
}

void GLSLShader::Enable()
{
    if(valid)
    {
        glUseProgramObjectARB(shader);
        enabled = true;
    }
}

void GLSLShader::Disable()
{
    glUseProgramObjectARB(0);
    enabled = false;
}

bool GLSLShader::AddAttribute(std::string name, ParameterType type)
{
    GLSLAttribute att;
    att.name = name;
    att.type = type;
    
    Enable();
    att.index = glGetAttribLocationARB(shader, name.c_str());
    Disable();
    
    if(att.index < 0)
        return false;
    
    attributes.push_back(att);
    return true;
}

bool GLSLShader::AddUniform(std::string name, ParameterType type)
{
    GLSLUniform uni;
    uni.name = name;
    uni.type = type;
    
    Enable();
    uni.location = glGetUniformLocationARB(shader, name.c_str());
    Disable();
    
    if(uni.location < 0)
        return false;
    
    uniforms.push_back(uni);
    return true;
}

bool GLSLShader::SetAttribute(std::string name, GLfloat x)
{
    GLint index = 0;
    bool success = GetAttribute(name, FLOAT, index);
    
    if(success)
        glVertexAttrib1fARB(index, x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, bool x)
{
    GLint location = 0;
    bool success = GetUniform(name, BOOLEAN, location);
  
    if(success)
        glUniform1iARB(location, (GLint)x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, GLfloat x)
{
    GLint location = 0;
    bool success = GetUniform(name, FLOAT, location);
    
    if(success)
        glUniform1fARB(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::vec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC2, location);
    
    if(success)
        glUniform2fvARB(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::vec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC3, location);
    
    if(success)
        glUniform3fvARB(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::vec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC4, location);
    
    if(success)
        glUniform4fvARB(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, GLint x)
{
    GLint location = 0;
    bool success = GetUniform(name, INT, location);
    
    if(success)
        glUniform1iARB(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::ivec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC2, location);
    
    if(success)
        glUniform2ivARB(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::ivec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC3, location);
    
    if(success)
        glUniform3ivARB(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::ivec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC4, location);
    
    if(success)
        glUniform4ivARB(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::mat3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, MAT3, location);
    
    if(success)
        glUniformMatrix3fvARB(location, 1, GL_FALSE, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::mat4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, MAT4, location);
    
    if(success)
        glUniformMatrix4fvARB(location, 1, GL_FALSE, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::GetUniform(std::string name, ParameterType type, GLint& location)
{
    for(int i = 0; i < uniforms.size(); i++)
        if(uniforms[i].name == name)
        {
            if(uniforms[i].type == type)
            {
                location = uniforms[i].location;
                return true;
            }
            else
            {
                cError("Uniform %s doesn't exist! Mismatched type!", name.c_str());
                return false;
            }
        }

    cError("Uniform %s doesn't exist!", name.c_str());
    return false;
}

bool GLSLShader::GetAttribute(std::string name, ParameterType type, GLint& index)
{
    for(int i = 0; i < attributes.size(); i++)
        if(attributes[i].name == name)
        {
            if(attributes[i].type == type)
            {
                index = attributes[i].index;
                return true;
            }
            else
                return false;
        }
    
    return false;
}

//// Statics
bool GLSLShader::Init()
{
    if(!CheckShadersAvailable())
        return false;
    
    GLint compiled;
    saqVertexShader = LoadShader(GL_VERTEX_SHADER, "saq.vert", &compiled);
    return true;
}

void GLSLShader::Destroy()
{
    if(saqVertexShader != NULL)
        glDeleteObjectARB(saqVertexShader);
}

//// Private statics
bool GLSLShader::CheckShadersAvailable()
{
    bool exts[4];
    exts[0] = CheckForExtension("GL_ARB_shader_objects");
    exts[1] = CheckForExtension("GL_ARB_shading_language_100");
    exts[2] = CheckForExtension("GL_ARB_vertex_shader");
    exts[3] = CheckForExtension("GL_ARB_fragment_shader");
    return exts[0] && exts[1] && exts[2] && exts[3];
}

GLhandleARB GLSLShader::LoadShader(GLenum shaderType, const char *filename, GLint *shaderCompiled)
{
	GLhandleARB shaderObject = NULL;
    
    char path[1024];
    GetShaderPath(path, 1024-32);
    strcat(path, filename);
    
    cInfo("Loading shader from: %s", path);
    
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
				
				cWarning("Shader compile log: %s", infoLog);
				free(infoLog);
			}
		}
		
		glGetObjectParameterivARB(shaderObject, GL_OBJECT_COMPILE_STATUS_ARB, shaderCompiled);
		
		if(*shaderCompiled == 0)
			cError("Failed to compile shader: %s", shaderSource);
        
        free(shaderSource);
	}
	else
		*shaderCompiled = 1;
    
	return shaderObject;
}

void GLSLShader::LinkProgram(GLhandleARB programObject, GLint *programLinked)
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
			
			cWarning("Program link log: %s", infoLog);
			free(infoLog);
		}
	}
	
	glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, programLinked);
	
	if(*programLinked == 0)
    {
		cError("Failed to link program: %s", (GLubyte *)&programObject);
    }
}

GLhandleARB GLSLShader::CreateProgramObject(GLhandleARB vertexShader, GLhandleARB fragmentShader)
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