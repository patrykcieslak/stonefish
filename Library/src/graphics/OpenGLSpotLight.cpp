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
//  OpenGLSpotLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSpotLight.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLCamera.h"

namespace sf
{

OpenGLSpotLight::OpenGLSpotLight(glm::vec3 position, glm::vec3 direction, GLfloat radius, GLfloat coneAngleDeg, glm::vec3 color, GLfloat lum) 
	: OpenGLLight(position, radius, color, lum)
{
    coneAngle = glm::max(0.1f, coneAngleDeg)/180.f*M_PI;
    GLfloat S = 2.f*M_PI*(1.f-cosf(coneAngle/2.f));
    colorLi = glm::vec4(color, lum/S);
    clipSpace = glm::mat4();
	GLfloat near = getSourceRadius()/tanf(coneAngle/2.f);
    zNear = glm::max(0.05f, near);
    zFar = sqrtf(colorLi.a/MIN_INTENSITY_THRESHOLD);

    Mesh* m = new Mesh;
    Vertex vt;
    Face f;
    m->hasUVs = false;
    unsigned int slices = 24;    
    vt.normal = glm::vec3(0,0,-1.f);
    vt.pos = glm::vec3(0,0,0);
    m->vertices.push_back(vt);
    //vertices
    for(unsigned int i=0; i<=slices; ++i)
    {
        vt.pos = glm::vec3(sinf(i/(GLfloat)slices*M_PI*2.f)*getSourceRadius(), -cosf(i/(GLfloat)slices*M_PI*2.f)*getSourceRadius(), 0.0);
        m->vertices.push_back(vt);
    }
    //faces
    for(unsigned int i=0; i<slices; ++i)
    {
        f.vertexID[0] = 0;
        f.vertexID[1] = i + 2;
        f.vertexID[2] = i + 1;
        m->faces.push_back(f);
    }
    sourceObject = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(m);
    delete m;
    
	UpdatePosition(position);
    UpdateDirection(direction);
    UpdateTransform();
}

OpenGLSpotLight::~OpenGLSpotLight()
{
    if(shadowFBO != 0) glDeleteFramebuffers(1, &shadowFBO);
}

void OpenGLSpotLight::InitShadowmap(GLint shadowmapLayer)
{
    //Create shadowmap framebuffer
    glGenFramebuffers(1, &shadowFBO);
    OpenGLState::BindFramebuffer(shadowFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, spotShadowArrayTex, 0, shadowmapLayer);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        printf("FBO initialization failed.\n");
    
    OpenGLState::BindFramebuffer(0);
}

LightType OpenGLSpotLight::getType() const
{
    return SPOT_LIGHT;
}

glm::vec3 OpenGLSpotLight::getDirection()
{
    return dir;
}

GLfloat OpenGLSpotLight::getAngle()
{
    return coneAngle;
}

glm::mat4 OpenGLSpotLight::getClipSpace()
{
    return clipSpace;
}

glm::mat4 OpenGLSpotLight::getTransform()
{
    glm::vec3 pos = getPosition() + getDirection() * (zNear - 1e-3f);
    return glm::inverse(glm::lookAt(pos, pos + getDirection(), glm::vec3(0,0,1.f)));
}
    
void OpenGLSpotLight::UpdateDirection(glm::vec3 d)
{
    tempDir = d;
}
    
void OpenGLSpotLight::UpdateTransform()
{
    OpenGLLight::UpdateTransform();
    dir = tempDir;
}

void OpenGLSpotLight::SetupShader(LightUBO* ubo)
{
    SpotLightUBO* spotUbo = (SpotLightUBO*)ubo;
    spotUbo->position = getPosition();
    spotUbo->color = glm::vec3(colorLi) * colorLi.a; 
    spotUbo->direction = getDirection();
    spotUbo->cone = (GLfloat)cosf(getAngle()/2.f);
    spotUbo->clipSpace = getClipSpace();
    spotUbo->frustumNear = zNear;
    spotUbo->frustumFar = zFar;
    spotUbo->radius = glm::vec3(0.1f,0.1f,getSourceRadius());
}

void OpenGLSpotLight::BakeShadowmap(OpenGLPipeline* pipe)
{
    glm::mat4 proj = glm::perspective((GLfloat)(2.f * coneAngle), 1.f, zNear, zFar);
    glm::mat4 view = glm::lookAt(getPosition(),
                                 getPosition() + getDirection(),
                                 glm::vec3(0,0,1.f));
    
    glm::mat4 bias(0.5f, 0.f, 0.f, 0.f,
                   0.f, 0.5f, 0.f, 0.f,
                   0.f, 0.f, 0.5f, 0.f,
                   0.5f, 0.5f, 0.5f, 1.f);
    clipSpace = bias * (proj * view);
    
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetProjectionMatrix(proj);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetViewMatrix(view);
    
    OpenGLState::BindFramebuffer(shadowFBO);
    OpenGLState::Viewport(0, 0, SPOT_LIGHT_SHADOWMAP_SIZE, SPOT_LIGHT_SHADOWMAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(4.0f, 32.0f);
    pipe->DrawObjects();
    //glDisable(GL_POLYGON_OFFSET_FILL);
    OpenGLState::BindFramebuffer(0);
}

void OpenGLSpotLight::ShowShadowMap(glm::vec4 rect)
{
    /*OpenGLState::DisableBlend();
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, shadowMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    OpenGLContent::getInstance()->DrawTexturedQuad(x, y, w, h, shadowMap);*/
}

}
