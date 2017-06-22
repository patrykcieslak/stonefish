//
//  GLSLShader.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GLSLShader__
#define __Stonefish_GLSLShader__

#include "OpenGLPipeline.h"

typedef enum {BOOLEAN, INT, FLOAT, VEC2, VEC3, VEC4, IVEC2, IVEC3, IVEC4, MAT3, MAT4} ParameterType;

struct GLSLUniform
{
    std::string name;
    ParameterType type;
    GLint location;
};

struct GLSLAttribute
{
    std::string name;
    ParameterType type;
    GLint index;
};

class GLSLShader
{
public:
    GLSLShader(const char* fragment, const char* vertex = NULL); //NULL means standard SAQ
    ~GLSLShader();
    
    void Use();
    
    bool AddAttribute(std::string name, ParameterType type);
    bool AddUniform(std::string name, ParameterType type);
    
    bool SetAttribute(std::string name, GLfloat x);
    
    bool SetUniform(std::string name, bool x);
    bool SetUniform(std::string name, GLfloat x);
    bool SetUniform(std::string name, glm::vec2 x);
    bool SetUniform(std::string name, glm::vec3 x);
    bool SetUniform(std::string name, glm::vec4 x);
    bool SetUniform(std::string name, GLint x);
    bool SetUniform(std::string name, glm::ivec2 x);
    bool SetUniform(std::string name, glm::ivec3 x);
    bool SetUniform(std::string name, glm::ivec4 x);
    bool SetUniform(std::string name, glm::mat3 x);
    bool SetUniform(std::string name, glm::mat4 x);
    
    bool isValid();
    
    static bool Init();
    static void Destroy();
    
private:
    bool GetAttribute(std::string name, ParameterType type, GLint& index);
    bool GetUniform(std::string name, ParameterType type, GLint& location);
    
    std::vector<GLSLAttribute> attributes;
    std::vector<GLSLUniform> uniforms;
    GLuint shader;
    bool valid;
    
    static GLuint saqVertexShader;
    static GLuint LoadShader(GLenum shaderType, const char* filename, GLint* shaderCompiled);
    static GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader);
};


#endif
