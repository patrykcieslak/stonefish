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
//  OpenGLState.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 7/03/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLState.h"

#include "core/SimulationApp.h"

namespace sf
{

GLuint OpenGLState::framebuffer = 0;
GLuint OpenGLState::vertexArray = 0;
GLuint OpenGLState::program = 0;
GLuint OpenGLState::uniformBuffer = 0;
GLint OpenGLState::viewport[4] = {0,0,0,0};
bool OpenGLState::depthTest = false;
bool OpenGLState::stencilTest = false;
bool OpenGLState::blend = false;
bool OpenGLState::cullFace = false;
GLuint OpenGLState::activeTexture = 0;
std::vector<std::pair<GLenum, GLuint>> OpenGLState::textures = std::vector<std::pair<GLuint, GLuint>>(0);
GLfloat OpenGLState::maxAniso = -1.f;

void OpenGLState::Init()
{
    GLint texUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texUnits);

    if(maxAniso < 0.f)
    {
        cInfo("Checking GPU capabilities...");
        GLint maxTexSize;
        GLint maxTexLayers;
        GLint maxUniforms;    
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTexLayers);
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxUniforms);
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        cInfo("Number of texture units available: %d", texUnits);
        cInfo("Maximum texture size: %d", maxTexSize);
        cInfo("Maximum number of texture layers: %d", maxTexLayers);
        cInfo("Maximum number of fragment shader uniforms: %d", maxUniforms);
        cInfo("Setting up OpenGL...");
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_CLAMP);
    glPointSize(1.f);
    glLineWidth(1.f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, new GLfloat[2]{1,1});
    glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, new GLfloat[4]{1,1,1,1});
    
	textures.clear();
    for(GLint i=0; i<texUnits; ++i)
        textures.push_back(std::make_pair(0,0));
}

void OpenGLState::BindFramebuffer(GLuint handle)
{
    if(framebuffer != handle)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
        framebuffer = handle;
    }
}

void OpenGLState::BindVertexArray(GLuint handle)
{
    if(vertexArray != handle)
    {
        glBindVertexArray(handle);
        vertexArray = handle;
    }
}

void OpenGLState::UseProgram(GLuint handle)
{
    if(program != handle)
    {
        glUseProgram(handle);
        program = handle;
    }
}

void OpenGLState::Viewport(GLint x, GLint y, GLuint width, GLuint height)
{
    if(viewport[2] != (GLint)width
       || viewport[3] != (GLint)height
       || viewport[0] != x
       || viewport[1] != y)
    {
        glViewport(x, y, width, height);
        viewport[0] = x;
        viewport[1] = y;
        viewport[2] = (GLint)width;
        viewport[3] = (GLint)height;
    }
}

void OpenGLState::EnableDepthTest()
{
    if(!depthTest)
    {
        glEnable(GL_DEPTH_TEST);
        depthTest = true;
    }
}

void OpenGLState::DisableDepthTest()
{
    if(depthTest)
    {
        glDisable(GL_DEPTH_TEST);
        depthTest = false;
    }
}

void OpenGLState::EnableStencilTest()
{
    if(!stencilTest)
    {
        glEnable(GL_STENCIL_TEST);
        stencilTest = true;
    }
}

void OpenGLState::DisableStencilTest()
{
    if(stencilTest)
    {
        glDisable(GL_STENCIL_TEST);
        stencilTest = false;
    }
}

void OpenGLState::EnableBlend()
{
    if(!blend)
    {
        glEnable(GL_BLEND);
        blend = true;
    }
}

void OpenGLState::DisableBlend()
{
    if(blend)
    {
        glDisable(GL_BLEND);
        blend = false;
    }
}

void OpenGLState::EnableCullFace()
{
    if(!cullFace)
    {
        glEnable(GL_CULL_FACE);
        cullFace = true;
    }
}

void OpenGLState::DisableCullFace()
{
    if(cullFace)
    {
        glDisable(GL_CULL_FACE);
        cullFace = false;
    }
}

void OpenGLState::BindTexture(GLuint unit, GLenum type, GLuint handle)
{
    if(unit >= textures.size())
        return;
    
    if(textures[unit].first == type && textures[unit].second == handle)
        return;
    
    if(activeTexture != unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        activeTexture = unit;
    }
    
    glBindTexture(type, handle);
    textures[unit].first = type;
    textures[unit].second = handle;
}

void OpenGLState::UnbindTexture(GLuint unit)
{
    if(unit >= textures.size())
        return;
    
    if(textures[unit].second == 0)
        return;
    
    if(activeTexture != unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        activeTexture = unit;
    }
    
    glBindTexture(textures[unit].first, 0);
    textures[unit].second = 0;
}

GLfloat OpenGLState::GetMaxAnisotropy()
{
    return maxAniso;
}

OpenGLState::OpenGLState() noexcept
{
}

}
