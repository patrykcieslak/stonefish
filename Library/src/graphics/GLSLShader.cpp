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
//  GLSLShader.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/05/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/GLSLShader.h"

#include <fstream>
#include "core/SimulationApp.h"
#include "graphics/OpenGLState.h"
#include "utils/SystemUtil.hpp"
#ifdef EMBEDDED_RESOURCES
#include <sstream>
#include "ResourceHandle.h"
#endif

namespace sf
{

GLuint GLSLShader::saqVertexShader = 0;
bool GLSLShader::verbose = true;

GLSLShader::GLSLShader(const std::vector<GLSLSource>& sources, const std::vector<GLuint>& precompiled)
{
    valid_ = false;
    program_ = 0;

    if(sources.size() > 0)
    {
        valid_ = true;
        
        std::vector<GLuint> shaders = precompiled;
        GLint compiled = 0;

        for(size_t i=0; i<sources.size(); ++i)
        {
            GLuint shader = LoadShader(sources[i].type, sources[i].filename, sources[i].header, &compiled);
            if(compiled == 0)
            {
                valid_ = false;
                break;
            }
            shaders.push_back(shader);
        }

        if(valid_)
        {
            program_ = CreateProgram(shaders, precompiled.size());
            if(program_ == 0)
                valid_ = false;
        }
    }
}

GLSLShader::GLSLShader(const std::vector<GLuint>& precompiled)
{
    program_ = CreateProgram(precompiled, precompiled.size());
    if(program_ == 0)
        valid_ = false;
    else 
        valid_ = true;
}

GLSLShader::GLSLShader(const std::string& fragment, const std::string& vertex)
{
    valid_ = false;
    GLint compiled = 0;
    GLuint vs;
    GLuint fs;
    std::string emptyHeader = "";
    
    if(vertex == "")
        vs = saqVertexShader;
    else
        vs = LoadShader(GL_VERTEX_SHADER, vertex, emptyHeader, &compiled);
    
    fs = LoadShader(GL_FRAGMENT_SHADER, fragment, emptyHeader, &compiled);
    program_ = CreateProgram({vs, fs}, vs == saqVertexShader ? 1 : 0);
    valid_ = true;
}
    
GLSLShader::~GLSLShader()
{
    if(valid_)
        glDeleteProgram(program_);
}

bool GLSLShader::isValid()
{
    return valid_;
}

GLuint GLSLShader::getProgramHandle()
{
    return program_;
}

void GLSLShader::Use()
{
    if(valid_)
        OpenGLState::UseProgram(program_);
#ifdef DEBUG
    else
        cError("Trying to use invalid shader!");
#endif
}

bool GLSLShader::AddAttribute(const std::string& name, ParameterType type)
{
    GLSLAttribute att;
    att.name = name;
    att.type = type;
    
    Use();
    att.index = glGetAttribLocation(program_, name.c_str());
    OpenGLState::UseProgram(0);
    
    if(att.index < 0)
        return false;
    
    attributes_.push_back(att);
    return true;
}

bool GLSLShader::AddUniform(const std::string& name, ParameterType type)
{
    GLSLUniform uni;
    uni.name = name;
    uni.type = type;
    
    Use();
    uni.location = glGetUniformLocation(program_, name.c_str());
    OpenGLState::UseProgram(0);
    
    if(uni.location < 0)
    {
#ifdef DEBUG
        //cError("Uniform '%s' doesn't exist!", name.c_str());
#endif
        return false;
    }
    
    uniforms_.push_back(uni);
    return true;
}

bool GLSLShader::SetAttribute(const std::string& name, GLfloat x)
{
    GLint index = 0;
    bool success = GetAttribute(name, FLOAT, index);
    
    if(success)
        glVertexAttrib1f(index, x);
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, bool x)
{
    GLint location = 0;
    bool success = GetUniform(name, BOOLEAN, location);
  
    if(success)
        glUniform1i(location, (GLint)x);
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, GLfloat x)
{
    GLint location = 0;
    bool success = GetUniform(name, FLOAT, location);
    
    if(success)
        glUniform1f(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::vec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC2, location);
    
    if(success)
        glUniform2fv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::vec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC3, location);
    
    if(success)
        glUniform3fv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::vec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, VEC4, location);
    
    if(success)
        glUniform4fv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, GLuint x)
{
    GLint location = 0;
    bool success = GetUniform(name, UINT, location);
    
    if(success)
        glUniform1ui(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, GLint x)
{
    GLint location = 0;
    bool success = GetUniform(name, INT, location);
    
    if(success)
        glUniform1i(location, x);
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::ivec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC2, location);
    
    if(success)
        glUniform2iv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::ivec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC3, location);
    
    if(success)
        glUniform3iv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::ivec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, IVEC4, location);
    
    if(success)
        glUniform4iv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::uvec2 x)
{
    GLint location = 0;
    bool success = GetUniform(name, UVEC2, location);
    
    if(success)
        glUniform2uiv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::uvec3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, UVEC3, location);
    
    if(success)
        glUniform3uiv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::uvec4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, UVEC4, location);
    
    if(success)
        glUniform4uiv(location, 1, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::mat3 x)
{
    GLint location = 0;
    bool success = GetUniform(name, MAT3, location);
    
    if(success)
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::SetUniform(const std::string& name, glm::mat4 x)
{
    GLint location = 0;
    bool success = GetUniform(name, MAT4, location);
    
    if(success)
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(x));
    
    return success;
}

bool GLSLShader::GetUniform(const std::string& name, ParameterType type, GLint& location)
{
    for(unsigned int i = 0; i < uniforms_.size(); i++)
        if(uniforms_[i].name == name)
        {
            if(uniforms_[i].type == type)
            {
                location = uniforms_[i].location;
                return true;
            }
            else
            {
#ifdef DEBUG
                cError("Uniform %s doesn't exist! Mismatched type!", name.c_str());
#endif
                return false;
            }
        }

    //cError("Uniform %s doesn't exist!", name.c_str());
    return false;
}

bool GLSLShader::GetAttribute(const std::string& name, ParameterType type, GLint& index)
{
    for(unsigned int i = 0; i < attributes_.size(); i++)
        if(attributes_[i].name == name)
        {
            if(attributes_[i].type == type)
            {
                index = attributes_[i].index;
                return true;
            }
            else
                return false;
        }
    
    return false;
}

bool GLSLShader::BindUniformBlock(const std::string& name, GLuint bindingPoint)
{
    GLuint blockIndex = glGetUniformBlockIndex(program_, name.c_str());
    if(blockIndex != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(program_, blockIndex, bindingPoint);
        return true;
    }
    else
    {
#ifdef DEBUG
        cError("Uniform block %s not found in program!", name.c_str());
#endif
        return false;
    }
}

bool GLSLShader::BindShaderStorageBlock(const std::string& name, GLuint bindingPoint)
{
    GLuint blockIndex = glGetProgramResourceIndex(program_, GL_SHADER_STORAGE_BLOCK, name.c_str());
    if(blockIndex != GL_INVALID_INDEX)
    {
        glShaderStorageBlockBinding(program_, blockIndex, bindingPoint);
        return true;
    }
    else
    {
#ifdef DEBUG
        cError("Shader storage block %s not found in program!", name.c_str());
#endif
        return false;
    }
}

//// Statics
bool GLSLShader::Init()
{
    GLint compiled;
    std::string emptyHeader = "";
    saqVertexShader = LoadShader(GL_VERTEX_SHADER, "saq.vert", emptyHeader, &compiled);
    return compiled;
}

void GLSLShader::Destroy()
{
    if(saqVertexShader != 0)
        glDeleteProgram(saqVertexShader);
}

void GLSLShader::Silent()
{
    verbose = false;
}

void GLSLShader::Verbose()
{
    verbose = true;
}

GLuint GLSLShader::LoadShader(GLenum shaderType, const std::string& filename, const std::string& header, GLint *shaderCompiled)
{
    GLuint shader = 0;
    std::string sourcePath = GetShaderPath() + filename;
#ifdef EMBEDDED_RESOURCES
    ResourceHandle rh(sourcePath);
    if(!rh.isValid())
    {
        cCritical("Shader resource not found: %s", sourcePath.c_str());
        return 0;
    }
    std::istringstream sourceString(rh.string());
    std::istream& sourceBuf(sourceString);
#else
    std::ifstream sourceFile(sourcePath);
    if(!sourceFile.is_open())
    {
        cCritical("Shader file not found: %s", sourcePath.c_str());
        return 0;
    }
    std::istream& sourceBuf(sourceFile);
#endif
#ifdef DEBUG
    if(verbose)
        cInfo("Loading shader from: %s", sourcePath.c_str());
#endif
    std::string source = header + "\n";
    std::string line;
    while(!sourceBuf.eof())
    {
        std::getline(sourceBuf, line);
        
        if(line.find("#inject") == std::string::npos)
            source.append(line + "\n");
        else //inject code from another source file
        {
            size_t pos1 = line.find("\"");
            size_t pos2 = line.find("\"", pos1+1);
            
            if(pos1 > 0 && pos2 > pos1)
            {
                std::string injectedPath = GetShaderPath() + line.substr(pos1+1, pos2-pos1-1);
#ifdef EMBEDDED_RESOURCES
                ResourceHandle rh2(injectedPath);
                if(!rh2.isValid())
                {
                    cCritical("Shader include resource not found: %s", injectedPath.c_str());
                    return 0;
                }
                std::istringstream injectedString(rh2.string());
                std::istream& injectedBuf(injectedString);
#else
                
                std::ifstream injectedFile(injectedPath);
                if(!injectedFile.is_open())
                {
                    sourceFile.close();
                    cCritical("Shader include file not found: %s", injectedPath.c_str());
                    return 0;
                }
                std::istream& injectedBuf(injectedFile);
#endif
#ifdef DEBUG
                if(verbose)
                    cInfo("--> Injecting source from: %s", injectedPath.c_str());
#endif
                while(!injectedBuf.eof())
                {
                    std::getline(injectedBuf, line);
                    source.append(line + "\n");
                }
#ifndef EMBEDDED_RESOURCES
                injectedFile.close();
#endif
            }
        }
    }
#ifndef EMBEDDED_RESOURCES
    sourceFile.close();
#endif    
    const char* shaderSource = source.c_str();
    if(shaderSource != NULL)
    {
        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
        if(*shaderCompiled == 0)
        {
            cError("Failed to compile shader: %s", sourcePath.c_str());
#ifdef DEBUG	
            GLint infoLogLength = 0;	
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            if(infoLogLength > 0)
            {
                std::vector<char> infoLog(infoLogLength+1);
                glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);
                cWarning("Shader compile log: %s", &infoLog[0]);
            }
#endif
            shader = 0;
        }
    }
    else
        *shaderCompiled = 0;
    
    return shader;
}

GLuint GLSLShader::CreateProgram(const std::vector<GLuint>& compiledShaders, unsigned int doNotDeleteNFirstShaders)
{
    GLint programLinked = 0;
    GLuint program = glCreateProgram();
    
    for(unsigned int i=0; i<compiledShaders.size(); ++i)
        if(compiledShaders[i] > 0)
            glAttachShader(program, compiledShaders[i]);
    
    glLinkProgram(program);
#ifdef DEBUG
    GLint infoLogLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        std::vector<char> infoLog(infoLogLength+1);
        glGetProgramInfoLog(program, infoLogLength, NULL, &infoLog[0]);
        cWarning("Program link log: %s", &infoLog[0]);
    }
#endif
    glGetProgramiv(program, GL_LINK_STATUS, &programLinked);
    
    for(unsigned int i=0; i<compiledShaders.size(); ++i)
    {
        if(compiledShaders[i] > 0)
        {
            glDetachShader(program, compiledShaders[i]);
            if(i > doNotDeleteNFirstShaders-1)
                glDeleteShader(compiledShaders[i]);
        }
    }
    
    if(programLinked == 0)
    {
        cError("Failed to link program!");
        glDeleteProgram(program);
        program = 0;
    }
    
    return program;
}

}
