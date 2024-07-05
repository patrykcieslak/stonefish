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
//  OpenGLParticleSystem.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2024 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLParticleSystem.h"

#include "graphics/OpenGLState.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

const GLuint OpenGLParticleSystem::noiseSize = 16;
GLuint OpenGLParticleSystem::noiseTexture = 0;

OpenGLParticleSystem::OpenGLParticleSystem(GLuint maxParticles) 
    : maxParticles(maxParticles), uniformDist(0, 1.f), normalDist(0, 1.f), initialized(false)
{
}

OpenGLParticleSystem::~OpenGLParticleSystem()
{
}
    
void OpenGLParticleSystem::Init()
{
    unsigned int seed = (unsigned int)GetTimeInMicroseconds();
    std::mt19937 randGen(seed);
    std::uniform_int_distribution<int8_t> dist(-127,127);
    glm::uvec3 noiseSize3(noiseSize, noiseSize, noiseSize);
    int8_t* noiseData = new int8_t[noiseSize3.x * noiseSize3.y * noiseSize3.z * 4];
    int8_t *ptr = noiseData;
    for(unsigned int z=0; z<noiseSize3.z; ++z)
        for(unsigned int y=0; y<noiseSize3.y; ++y) 
            for(unsigned int x=0; x<noiseSize3.x; ++x) 
            {
              *ptr++ = dist(randGen);
              *ptr++ = dist(randGen);
              *ptr++ = dist(randGen);
              *ptr++ = dist(randGen);
            }
    noiseTexture = OpenGLContent::GenerateTexture(GL_TEXTURE_3D, noiseSize3,
        GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, noiseData, FilteringMode::BILINEAR, true);
    delete [] noiseData;                                
}

void OpenGLParticleSystem::Destroy()
{
    if(noiseTexture != 0) 
        glDeleteTextures(1, &noiseTexture);
}

}
