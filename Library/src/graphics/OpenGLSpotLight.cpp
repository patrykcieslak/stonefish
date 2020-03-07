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
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSpotLight.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

OpenGLSpotLight::OpenGLSpotLight(glm::vec3 position, glm::vec3 direction, GLfloat coneAngleDeg, glm::vec3 color, GLfloat illuminance) : OpenGLLight(position, color, illuminance)
{
    coneAngle = coneAngleDeg/180.f*M_PI;
    clipSpace = glm::mat4();
    zNear = 0.1f;
    zFar = 100.f;
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

LightType OpenGLSpotLight::getType()
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
    
void OpenGLSpotLight::UpdateDirection(glm::vec3 d)
{
    tempDir = d;
}
    
void OpenGLSpotLight::UpdateTransform()
{
    OpenGLLight::UpdateTransform();
    dir = tempDir;
}

void OpenGLSpotLight::SetupShader(GLSLShader* shader, unsigned int lightId)
{
    std::string lightUni = "spotLights[" + std::to_string(lightId) + "].";
    shader->SetUniform(lightUni + "position", getPosition());
    shader->SetUniform(lightUni + "radius", glm::vec2(0.01f,0.01f));
    shader->SetUniform(lightUni + "color", getColor());
    shader->SetUniform(lightUni + "direction", getDirection());
    shader->SetUniform(lightUni + "angle", (GLfloat)cosf(getAngle()/2.f));
    shader->SetUniform(lightUni + "clipSpace", getClipSpace());
    shader->SetUniform(lightUni + "zNear", zNear);
    shader->SetUniform(lightUni + "zFar", zFar);
}

void OpenGLSpotLight::BakeShadowmap(OpenGLPipeline* pipe)
{
    OpenGLState::EnableDepthTest();
    OpenGLState::EnableCullFace();
    glCullFace(GL_FRONT);

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
