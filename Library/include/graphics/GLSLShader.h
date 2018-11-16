//
//  GLSLShader.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GLSLShader__
#define __Stonefish_GLSLShader__

#include <utility>
#include "graphics/OpenGLPipeline.h"

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

struct GLSLHeader
{
	std::string code;
	bool useInVertex;
	bool useInTessCtrl;
	bool useInTessEval;
	bool useInGeometry;
	bool useInFragment;
};

class GLSLShader
{
public:
    GLSLShader(std::string fragment,     //Obligatory
			   std::string vertex = "",  //Optional
			   std::string geometry = "", 
			   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
    GLSLShader(GLSLHeader& header, 
			   std::string fragment, 
			   std::string vertex = "", 
			   std::string geometry = "", 
			   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
	GLSLShader(std::vector<GLuint> compiledShaders,
			   std::string fragment = "",     
			   std::string vertex = "",
			   std::string geometry = "", 
			   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
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
	static void Silent();
	static void Verbose();
    
	static GLuint LoadShader(GLenum shaderType, std::string filename, std::string& header, GLint* shaderCompiled);
	
private:
    bool GetAttribute(std::string name, ParameterType type, GLint& index);
    bool GetUniform(std::string name, ParameterType type, GLint& location);
    
    std::vector<GLSLAttribute> attributes;
    std::vector<GLSLUniform> uniforms;
    GLuint shader;
    bool valid;
    
    static GLuint saqVertexShader;
    static bool verbose;
	static GLuint CreateProgram(const std::vector<GLuint>& compiledShaders, unsigned int doNotDeleteNFirstShaders = 0);
};


#endif
