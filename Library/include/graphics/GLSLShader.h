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
//  GLSLShader.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GLSLShader__
#define __Stonefish_GLSLShader__

#include <utility>
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum defining possible GLSL variable types.
    typedef enum {BOOLEAN, INT, FLOAT, VEC2, VEC3, VEC4, IVEC2, IVEC3, IVEC4, MAT3, MAT4} ParameterType;
    
    //! A structure containing information about a GLSL uniform.
    struct GLSLUniform
    {
        std::string name;
        ParameterType type;
        GLint location;
    };
    
    //! A structure containing information about a GLSL attribute.
    struct GLSLAttribute
    {
        std::string name;
        ParameterType type;
        GLint index;
    };
    
    //! A structure containing a header added to shaders during compilation.
    struct GLSLHeader
    {
        std::string code;
        bool useInVertex;
        bool useInTessCtrl;
        bool useInTessEval;
        bool useInGeometry;
        bool useInFragment;
		
		GLSLHeader(): 
			code(""), 
			useInVertex(false), 
			useInTessCtrl(false), 
			useInTessEval(false), 
			useInGeometry(false), 
			useInFragment(false)
		{};
    };
    
    //! A class representing a single GLSL shader.
    class GLSLShader
    {
    public:
        //! A constructor.
        /*!
         \param fragment path to the fragment shader
         \param vertex path to the vertex shader
         \param geometry path to the geometry shader
         \param tesselation a pair of two path to the tesselation shaders
         */
        GLSLShader(std::string fragment,
                   std::string vertex = "",
                   std::string geometry = "",
                   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
        
        //! A constructor.
        /*!
         \param header a reference to a header structure
         \param fragment path to the fragment shader
         \param vertex path to the vertex shader
         \param geometry path to the geometry shader
         \param tesselation a pair of two path to the tesselation shaders
         */
        GLSLShader(GLSLHeader& header,
                   std::string fragment,
                   std::string vertex = "",
                   std::string geometry = "",
                   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
        
        //! A constructor.
        /*!
         \param compiledShaders a list of compiled shaders that should be linked
         \param fragment path to the fragment shader
         \param vertex path to the vertex shader
         \param geometry path to the geometry shader
         \param tesselation a pair of two path to the tesselation shaders
         */
        GLSLShader(std::vector<GLuint> compiledShaders,
                   std::string fragment = "",
                   std::string vertex = "",
                   std::string geometry = "",
                   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
        
		//! A constructor.
        /*!
         \param compiledShaders a list of compiled shaders that should be linked
		 \param header a reference to a header structure
         \param fragment path to the fragment shader
         \param vertex path to the vertex shader
         \param geometry path to the geometry shader
         \param tesselation a pair of two path to the tesselation shaders
         */
        GLSLShader(std::vector<GLuint> compiledShaders,
				   GLSLHeader& header,
                   std::string fragment = "",
                   std::string vertex = "",
                   std::string geometry = "",
                   std::pair<std::string, std::string> tesselation = std::make_pair("",""));
				   
        //! A destructor.
        ~GLSLShader();
        
        //! A method engageing the shader.
        void Use();
        
        //! A method to define a GLSL attribute.
        /*!
         \param name the name of the attribute
         \param type the type of the attribute
         \return success
         */
        bool AddAttribute(std::string name, ParameterType type);
        
        //! A method to define a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param type the type of the uniform
         \return success
         */
        bool AddUniform(std::string name, ParameterType type);
        
        //! A method used to set a GLSL attribute.
        /*!
         \param name the name of the attribute
         \param x the value of the attribute
         \return success
         */
        bool SetAttribute(std::string name, GLfloat x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, bool x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, GLfloat x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::vec2 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::vec3 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::vec4 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, GLint x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::ivec2 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::ivec3 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::ivec4 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::mat3 x);
        
        //! A method used to set a GLSL uniform.
        /*!
         \param name the name of the uniform
         \param x the value of the uniform
         \return success
         */
        bool SetUniform(std::string name, glm::mat4 x);
        
        //! A method to check if the shader is valid.
        bool isValid();
        
        //! A method used to get the OpenGL program handle
        GLuint getProgramHandle();
        
        //! A static method to init shader environment.
        static bool Init();
        
        //! A static method to destroy common shader data.
        static void Destroy();
        
        //! A static method to enable silent shader compilation.
        static void Silent();
        
        //! A static method to enable verbose shader compilation.
        static void Verbose();
        
        //! A method that compiles a given shader from source.
        /*!
         \param shaderType a type of the shader
         \param filename the path to the shader source file
         \param header a reference to a header string
         \param shaderCompiled a pointer to a variable that will hold the shader compilation output code
         \return an id of the new compiled shader
         */
        static GLuint LoadShader(GLenum shaderType, std::string filename, const std::string& header, GLint* shaderCompiled);
        
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
}

#endif
