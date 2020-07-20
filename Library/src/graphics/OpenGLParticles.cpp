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
//  OpenGLParticles.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLParticles.h"

#include "graphics/OpenGLState.h"

namespace sf
{
    
OpenGLParticles::OpenGLParticles(GLuint numOfParticles)	
{
    nParticles = numOfParticles;
    glGenBuffers(1, &particlePosSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePosSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * nParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &particleVelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleVelSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * nParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &particleEAB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleEAB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6 * nParticles, NULL, GL_STATIC_DRAW);
    GLuint* indices = (GLuint*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * 6 * nParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for(GLuint i=0; i<nParticles; ++i) 
    {
        GLuint index = GLuint(i<<2);
        *(indices++) = index;
        *(indices++) = index+2;
        *(indices++) = index+1;
        *(indices++) = index;
        *(indices++) = index+3;
        *(indices++) = index+2;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glGenVertexArrays(1, &particleVAO);
    OpenGLState::BindVertexArray(particleVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleEAB);
    OpenGLState::BindVertexArray(0);
}

OpenGLParticles::~OpenGLParticles()
{
    glDeleteBuffers(1, &particlePosSSBO);
    glDeleteBuffers(1, &particleVelSSBO);
    glDeleteBuffers(1, &particleEAB);
    glDeleteVertexArrays(1, &particleVAO);
}
    
}
