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
//  OpenGLState.h
//  Stonefish
//
//  Created by Patryk Cieslak on 7/03/2020.
//  Copyright (c) 2020-2024 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLState__
#define __Stonefish_OpenGLState__

#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class OpenGLState
    {
    public:
        static void Init();
        static void BindFramebuffer(GLuint handle);
        static void BindVertexArray(GLuint handle);
        static void UseProgram(GLuint handle);
        //static void BindUniformBuffer(GLuint handle);
        static void Viewport(GLint x, GLint y, GLuint width, GLuint height);
        static void EnableDepthTest();
        static void DisableDepthTest();
        static void EnableStencilTest();
        static void DisableStencilTest();
        static void EnableBlend();
        static void DisableBlend();
        static void EnableCullFace();
        static void DisableCullFace();
        static void BindTexture(GLuint unit, GLenum type, GLuint handle);
        static void UnbindTexture(GLuint unit);
        static GLfloat GetMaxAnisotropy();
        static glm::uvec3 GetMaxTextureSize();
        
    private:
        OpenGLState() noexcept;
        
        static GLuint framebuffer;
        static GLuint vertexArray;
        static GLuint program;
        static GLuint uniformBuffer;
        static GLint viewport[4];
        static bool depthTest; //GL_DEPTH_TEST
        static bool stencilTest; //GL_STENCIL_TEST
        static bool blend; //GL_BLEND
        static bool cullFace; //GL_CULL_FACE
        static GLuint activeTexture; //Active texture unit
        static std::vector<std::pair<GLenum, GLuint>> textures; //(type, handle)
        static GLfloat maxAniso;
        static GLint maxTextureSize;
        static GLint maxTextureLayers;
    };
}

#endif
