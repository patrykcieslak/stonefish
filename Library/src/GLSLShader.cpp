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

GLuint GLSLShader::saqVertexShader = 0;

GLSLShader::GLSLShader(const char* fragment, const char* vertex)
{
    valid = false;
    enabled = false;
    GLint compiled = 0;
    GLuint vs;
    GLuint fs;
    
    if(vertex == NULL)
        vs = saqVertexShader;
    else
        vs = LoadShader(GL_VERTEX_SHADER, vertex, &compiled);
    
    fs = LoadShader(GL_FRAGMENT_SHADER, fragment, &compiled);
    
    shader = CreateProgram(vs, fs);
    valid = true;
}

GLSLShader::~GLSLShader()
{
    if(valid)
        glDeleteProgram(shader);
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
        glUseProgram(shader);
        enabled = true;
    }
}

void GLSLShader::Disable()
{
    glUseProgram(0);
    enabled = false;
}

bool GLSLShader::AddAttribute(std::string name, ParameterType type)
{
    GLSLAttribute att;
    att.name = name;
    att.type = type;
    
    Enable();
    att.index = glGetAttribLocation(shader, name.c_str());
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
    uni.location = glGetUniformLocation(shader, name.c_str());
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
        glVertexAttrib1f(index, x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, bool x)
{
    GLint location = 0;
    bool success = GetUniform(name, BOOLEAN, location);
  
    if(success)
        glUniform1i(location, (GLint)x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, GLfloat x)
{
    GLint location = 0;
    bool success = GetUniform(name, FLOAT, location);
    
    if(success)
        glUniform1f(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::vec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC2, location);
    
    if(success)
        glUniform2fv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::vec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC3, location);
    
    if(success)
        glUniform3fv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::vec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC4, location);
    
    if(success)
        glUniform4fv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, GLint x)
{
    GLint location = 0;
    bool success = GetUniform(name, INT, location);
    
    if(success)
        glUniform1i(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::ivec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC2, location);
    
    if(success)
        glUniform2iv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::ivec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC3, location);
    
    if(success)
        glUniform3iv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::ivec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC4, location);
    
    if(success)
        glUniform4iv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::mat3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, MAT3, location);
    
    if(success)
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(std::string name, glm::mat4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, MAT4, location);
    
    if(success)
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(x));
    
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

    //cError("Uniform %s doesn't exist!", name.c_str());
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
    GLint compiled;
    saqVertexShader = LoadShader(GL_VERTEX_SHADER, "saq.vert", &compiled);
    return compiled;
}

void GLSLShader::Destroy()
{
    if(saqVertexShader != 0)
        glDeleteProgram(saqVertexShader);
}

GLuint GLSLShader::LoadShader(GLenum shaderType, const char *filename, GLint *shaderCompiled)
{
	GLuint shader = 0;
    
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
		shader = glCreateShader(shaderType);
		
		glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
		glCompileShader(shader);
		
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		if(infoLogLength > 0)
		{
			std::vector<char> infoLog(infoLogLength+1);
			glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);
			cWarning("Shader compile log: %s", &infoLog[0]);
		}
		
		glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
		
		if(*shaderCompiled == 0)
			cError("Failed to compile shader: %s", shaderSource);
        
        free(shaderSource);
	}
	else
		*shaderCompiled = 0;
    
	return shader;
}

GLuint GLSLShader::CreateProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLint programLinked = 0;
	GLuint program = glCreateProgram();
	GLint infoLogLength = 0;
		
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength > 0)
	{
		std::vector<char> infoLog(infoLogLength+1);
		glGetProgramInfoLog(program, infoLogLength, NULL, &infoLog[0]);
		cWarning("Program link log: %s", &infoLog[0]);
	}
	
	glGetProgramiv(program, GL_LINK_STATUS, &programLinked);
	
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	
	if(vertexShader != saqVertexShader)
		glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	if(programLinked == 0)
    {
		cError("Failed to link program: %s", (GLuint*)&program);
		glDeleteProgram(program);
		program = 0;
	}
	
	return program;
}