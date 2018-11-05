//
//  GLSLShader.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include <graphics/GLSLShader.h>

#include <graphics/Console.h>
#include <utils/SystemUtil.hpp>
#include <fstream>

GLuint GLSLShader::saqVertexShader = 0;
bool GLSLShader::verbose = true;

GLSLShader::GLSLShader(std::string fragment, std::string vertex, std::string geometry, std::pair<std::string, std::string> tesselation)
{
    valid = false;
    GLint compiled = 0;
    GLuint vs;
	GLuint tcs;
	GLuint tes;
    GLuint gs;
	GLuint fs;
	std::string emptyHeader = "";
    
    if(vertex == "")
        vs = saqVertexShader;
    else
        vs = LoadShader(GL_VERTEX_SHADER, vertex, emptyHeader, &compiled);
    
	if(tesselation.first == "" || tesselation.second == "")
	{
		tcs = 0;
		tes = 0;
	}
	else
	{
		tcs = LoadShader(GL_TESS_CONTROL_SHADER, tesselation.first, emptyHeader, &compiled);
		tes = LoadShader(GL_TESS_EVALUATION_SHADER, tesselation.second, emptyHeader, &compiled);
	}
	
	if(geometry == "")
		gs = 0;
	else
		gs = LoadShader(GL_GEOMETRY_SHADER, geometry, emptyHeader, &compiled);
	
    fs = LoadShader(GL_FRAGMENT_SHADER, fragment, emptyHeader, &compiled);
    
	shader = CreateProgram({vs, gs, fs, tcs, tes}, vs == saqVertexShader ? 1 : 0);
		
    valid = true;
}

GLSLShader::GLSLShader(GLSLHeader& header, std::string fragment, std::string vertex, std::string geometry,  std::pair<std::string, std::string> tesselation)
{
	valid = false;
    GLint compiled = 0;
    GLuint vs;
	GLuint tcs;
	GLuint tes;
    GLuint gs;
	GLuint fs;
	std::string emptyHeader = "";
    
    if(vertex == "")
        vs = saqVertexShader;
    else
        vs = LoadShader(GL_VERTEX_SHADER, vertex, header.useInVertex ? header.code : emptyHeader, &compiled);
    
	if(tesselation.first == "" || tesselation.second == "")
	{
		tcs = 0;
		tes = 0;
	}
	else
	{
		tcs = LoadShader(GL_TESS_CONTROL_SHADER, tesselation.first, header.useInTessCtrl ? header.code : emptyHeader, &compiled);
		tes = LoadShader(GL_TESS_EVALUATION_SHADER, tesselation.second, header.useInTessEval ? header.code : emptyHeader, &compiled);
	}
	
	if(geometry == "")
		gs = 0;
	else
		gs = LoadShader(GL_GEOMETRY_SHADER, geometry, header.useInGeometry ? header.code : emptyHeader, &compiled);
	
    fs = LoadShader(GL_FRAGMENT_SHADER, fragment, header.useInFragment ? header.code : emptyHeader, &compiled);
    
	shader = CreateProgram({vs, gs, fs, tcs, tes}, vs == saqVertexShader ? 1 : 0);
		
    valid = true;
}

GLSLShader::GLSLShader(std::vector<GLuint> compiledShaders, std::string fragment, std::string vertex, std::string geometry,  std::pair<std::string, std::string> tesselation)
{
	valid = false;
    GLint compiled = 0;
    GLuint vs;
	GLuint tcs;
	GLuint tes;
    GLuint gs;
	GLuint fs;
	std::string emptyHeader = "";
    
    if(vertex == "")
		vs = 0;
	else
		vs = LoadShader(GL_VERTEX_SHADER, vertex, emptyHeader, &compiled);
    
	if(tesselation.first == "" || tesselation.second == "")
	{
		tcs = 0;
		tes = 0;
	}
	else
	{
		tcs = LoadShader(GL_TESS_CONTROL_SHADER, tesselation.first, emptyHeader, &compiled);
		tes = LoadShader(GL_TESS_EVALUATION_SHADER, tesselation.second, emptyHeader, &compiled);
	}
	
	if(geometry == "")
		gs = 0;
	else
		gs = LoadShader(GL_GEOMETRY_SHADER, geometry, emptyHeader, &compiled);
	
    fs = LoadShader(GL_FRAGMENT_SHADER, fragment, emptyHeader, &compiled);
    
	std::vector<GLuint> cShaders = compiledShaders;
	cShaders.push_back(vs);
	cShaders.push_back(gs);
	cShaders.push_back(fs);
	cShaders.push_back(tcs);
	cShaders.push_back(tes);	
	shader = CreateProgram(cShaders, vs == saqVertexShader ? compiledShaders.size() + 1 : compiledShaders.size());
		
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

void GLSLShader::Use()
{
    if(valid)
        glUseProgram(shader);
}

bool GLSLShader::AddAttribute(std::string name, ParameterType type)
{
    GLSLAttribute att;
    att.name = name;
    att.type = type;
    
    Use();
    att.index = glGetAttribLocation(shader, name.c_str());
    glUseProgram(0);
    
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
    
    Use();
    uni.location = glGetUniformLocation(shader, name.c_str());
	glUseProgram(0);
    
    if(uni.location < 0)
	{
#ifdef DEBUG
		cError("Uniform '%s' doesn't exist!", name.c_str());
#endif
        return false;
	}
    
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
    for(unsigned int i = 0; i < uniforms.size(); i++)
        if(uniforms[i].name == name)
        {
            if(uniforms[i].type == type)
            {
                location = uniforms[i].location;
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

bool GLSLShader::GetAttribute(std::string name, ParameterType type, GLint& index)
{
    for(unsigned int i = 0; i < attributes.size(); i++)
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

GLuint GLSLShader::LoadShader(GLenum shaderType, std::string filename, std::string& header, GLint *shaderCompiled)
{
	GLuint shader = 0;
    
	std::string basePath = GetShaderPath();
	std::string sourcePath = basePath + filename;
	std::ifstream sourceFile(sourcePath);
	std::string source = header + "\n";
	std::string line;

#ifdef DEBUG
	if(verbose)
		cInfo("Loading shader from: %s", sourcePath.c_str());
#endif
	while(!sourceFile.eof())
	{
		std::getline(sourceFile, line);
		
		if(line.find("#inject") == std::string::npos)
			source.append(line + "\n");
		else //inject code from another source file
		{
			size_t pos1 = line.find("\"");
			size_t pos2 = line.find("\"", pos1+1);
			
			if(pos1 > 0 && pos2 > pos1)
			{
				std::string injectedPath = basePath + line.substr(pos1+1, pos2-pos1-1);
				std::ifstream injectedFile(injectedPath);
#ifdef DEBUG
				if(verbose)
					cInfo("--> Injecting source from: %s", injectedPath.c_str());
#endif
				while(!injectedFile.eof())
				{
					std::getline(injectedFile, line);
					source.append(line + "\n");
				}
				injectedFile.close();
			}
		}
	}
	sourceFile.close();
	
	const char* shaderSource = source.c_str();
	
	if(shaderSource != NULL)
	{
		shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
		glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
		if(*shaderCompiled == 0)
			cError("Failed to compile shader: %s", shaderSource);
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
		cError("Failed to link program: %s", (GLuint*)&program);
		glDeleteProgram(program);
		program = 0;
	}
	
	return program;
}