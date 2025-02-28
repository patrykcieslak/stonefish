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
//  OpenGLLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLLight.h"

#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLRealCamera.h"
#include "graphics/OpenGLOcean.h"
#include "entities/forcefields/Ocean.h"

namespace sf
{

GLuint OpenGLLight::spotShadowArrayTex = 0;
GLuint OpenGLLight::spotDepthSampler = 0;
GLuint OpenGLLight::spotShadowSampler = 0;
OpenGLCamera* OpenGLLight::activeView = nullptr;

OpenGLLight::OpenGLLight(glm::vec3 position, GLfloat radius, glm::vec3 c, GLfloat lum)
{
    pos = tempPos = position;
	R = glm::max(0.01f, radius);
    active = true;
    sourceObject = -1;
}

OpenGLLight::~OpenGLLight()
{
}

bool OpenGLLight::isActive()
{
    return active;
}

glm::vec4 OpenGLLight::getColorLi()
{
    return colorLi;
}

glm::vec3 OpenGLLight::getPosition()
{
    return pos;
}

GLfloat OpenGLLight::getSourceRadius()
{
	return R;
}

int OpenGLLight::getSourceObject()
{
    return sourceObject;
}

glm::mat4 OpenGLLight::getTransform()
{
    return glm::translate(pos);
}

bool OpenGLLight::operator<(const OpenGLLight& l)
{
    return (int)getType() < (int)l.getType();
}

void OpenGLLight::UpdatePosition(glm::vec3 p)
{
    tempPos = p;
}

void OpenGLLight::UpdateTransform()
{
    pos = tempPos;
}

void OpenGLLight::SwitchOn()
{
    active = true;
}

void OpenGLLight::SwitchOff()
{
    active = false;
}

void OpenGLLight::InitShadowmap(GLint shadowmapLayer)
{
}
    
void OpenGLLight::BakeShadowmap(OpenGLPipeline* pipe)
{
}

void OpenGLLight::ShowShadowMap(glm::vec4 rect)
{
}

//////////////////static//////////////////////////////
void OpenGLLight::Init(std::vector<OpenGLLight*>& lights)
{
    if(lights.size() == 0)
        return;
    
    //Count spotlights
    unsigned int numOfSpotLights = 0;
    for(unsigned int i=0; i < lights.size(); ++i)
        if(lights[i]->getType() == LightType::SPOT) ++numOfSpotLights;
        
    //Generate shadowmap array
    glGenTextures(1, &spotShadowArrayTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D_ARRAY, spotShadowArrayTex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, SPOT_LIGHT_SHADOWMAP_SIZE, SPOT_LIGHT_SHADOWMAP_SIZE, numOfSpotLights, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    OpenGLState::UnbindTexture(TEX_BASE);
    
    //Generate samplers
    glGenSamplers(1, &spotDepthSampler);
    glSamplerParameteri(spotDepthSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(spotDepthSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(spotDepthSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(spotDepthSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenSamplers(1, &spotShadowSampler);
    glSamplerParameteri(spotShadowSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(spotShadowSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(spotShadowSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(spotShadowSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(spotShadowSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(spotShadowSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    
    //Initialize lights shadow FBOs
    numOfSpotLights = 0;
    for(unsigned int i=0; i < lights.size(); ++i)
        if(lights[i]->getType() == LightType::SPOT) lights[i]->InitShadowmap(numOfSpotLights++);		
        
    //Bind textures and samplers
    OpenGLState::BindTexture(TEX_SPOT_SHADOW, GL_TEXTURE_2D_ARRAY, spotShadowArrayTex);
    glBindSampler(TEX_SPOT_SHADOW, spotShadowSampler);
    
    OpenGLState::BindTexture(TEX_SPOT_DEPTH, GL_TEXTURE_2D_ARRAY, spotShadowArrayTex);
    glBindSampler(TEX_SPOT_DEPTH, spotDepthSampler);
}

void OpenGLLight::Destroy()
{
    if(spotShadowArrayTex != 0) glDeleteTextures(1, &spotShadowArrayTex);
    if(spotDepthSampler != 0) glDeleteSamplers(1, &spotDepthSampler);
    if(spotShadowSampler != 0) glDeleteSamplers(1, &spotShadowSampler);
}

void OpenGLLight::SetCamera(OpenGLCamera* view)
{
    activeView = view;
}

}



