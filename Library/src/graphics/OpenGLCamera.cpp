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
//  OpenGLCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLCamera.h"

#include <random>
#include "core/GraphicalSimulationApp.h"
#include "core/Console.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "entities/SolidEntity.h"

namespace sf
{

GLSLShader* OpenGLCamera::lightMeterShader = NULL;
GLSLShader* OpenGLCamera::tonemapShader = NULL;
GLSLShader* OpenGLCamera::depthLinearizeShader = NULL;
GLSLShader* OpenGLCamera::aoDeinterleaveShader = NULL;
GLSLShader* OpenGLCamera::aoCalcShader = NULL;
GLSLShader* OpenGLCamera::aoReinterleaveShader = NULL;
GLSLShader** OpenGLCamera::aoBlurShader = NULL;
GLSLShader* OpenGLCamera::ssrShader = NULL;
GLSLShader* OpenGLCamera::fxaaShader = NULL;

OpenGLCamera::OpenGLCamera(GLint x, GLint y, GLint width, GLint height, glm::vec2 range) : OpenGLView(x, y, width, height)
{
    fovx = 0.785f;
    near = range.x;
    far = range.y;
    activePostprocessTexture = 0;
    exposureComp = 0.f;
	autoExposure = true;
	toneMapping = true;
    antiAliasing = false;
    aoFactor = 0;
    
    if(GLAD_GL_VERSION_4_3 
       && ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getRenderSettings().ao != RenderQuality::QUALITY_DISABLED)
        aoFactor = 1;
    
    if(GLAD_GL_VERSION_4_2
       && ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getRenderSettings().aa)
        antiAliasing = true;
    
    //----Geometry rendering----
    glGenTextures(1, &renderColorTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, renderColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewportWidth, viewportHeight, 0, GL_RGB, GL_HALF_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &renderViewNormalTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, renderViewNormalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &renderDepthStencilTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, renderDepthStencilTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, viewportWidth, viewportHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenFramebuffers(1, &renderFBO);
    OpenGLState::BindFramebuffer(renderFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderColorTex, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, renderViewNormalTex, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, renderDepthStencilTex, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Render FBO initialization failed!");
        
    //----Light metering (automatic exposure)----
    glGenFramebuffers(1, &lightMeterFBO);
    OpenGLState::BindFramebuffer(lightMeterFBO);
    
    glGenTextures(1, &lightMeterTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, lightMeterTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 2, 2, 0, GL_RGB, GL_FLOAT, NULL); //Distribute work to 4 parallel threads
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Use hardware linear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightMeterTex, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Light meter FBO initialization failed!");
    
    //----Non-multisampled postprocessing----
    glGenFramebuffers(1, &postprocessFBO);
    OpenGLState::BindFramebuffer(postprocessFBO);
    
    glGenTextures(2, postprocessTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, postprocessTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewportWidth, viewportHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessTex[0], 0);
    
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, postprocessTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, postprocessTex[1], 0);
    
    glGenTextures(1, &postprocessStencilTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, postprocessStencilTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, viewportWidth, viewportHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, postprocessStencilTex, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Postprocess FBO initialization failed!");
    
    //Linear depth
    glGenTextures(2, linearDepthTex);
    //Front face
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, linearDepthTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth, viewportHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Back face
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, linearDepthTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth, viewportHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    OpenGLState::UnbindTexture(TEX_BASE);
    
    glGenFramebuffers(1, &linearDepthFBO);
    OpenGLState::BindFramebuffer(linearDepthFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, linearDepthTex[0], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, linearDepthTex[1], 0);
        
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("LinearDepth FBO initialization failed!");
    
    OpenGLState::BindFramebuffer(0);
    
    //----HBAO----
    if(aoFactor > 0)
    {
        //Deinterleaved results
        GLint swizzle[4] = {GL_RED,GL_GREEN,GL_ZERO,GL_ZERO};
        
        glGenTextures(1, &aoResultTex);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, aoResultTex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, viewportWidth, viewportHeight);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
        glGenTextures(1, &aoBlurTex);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, aoBlurTex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, viewportWidth, viewportHeight);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenFramebuffers(1, &aoFinalFBO);
        OpenGLState::BindFramebuffer(aoFinalFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoResultTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, aoBlurTex, 0);
        
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
            cError("AOFinal FBO initialization failed!");
        
        //Interleaved rendering
        int quarterWidth  = ((viewportWidth+3)/4);
        int quarterHeight = ((viewportHeight+3)/4);

        glGenTextures(1, &aoDepthArrayTex);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenTextures(1, &aoResultArrayTex);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D_ARRAY, aoResultArrayTex);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RG16F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        OpenGLState::UnbindTexture(TEX_BASE);
        
        GLenum drawbuffers[NUM_MRT];
        for(int layer = 0; layer < NUM_MRT; ++layer)
            drawbuffers[layer] = GL_COLOR_ATTACHMENT0 + layer;

        glGenFramebuffers(1, &aoDeinterleaveFBO);
        OpenGLState::BindFramebuffer(aoDeinterleaveFBO);
        glDrawBuffers(NUM_MRT, drawbuffers);
        
        glGenFramebuffers(1, &aoCalcFBO);
        OpenGLState::BindFramebuffer(aoCalcFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoResultArrayTex, 0);
        OpenGLState::BindFramebuffer(0);
        
        glGenBuffers(1, &aoDataUBO);
        glNamedBufferStorageEXT(aoDataUBO, sizeof(AOData), NULL, GL_DYNAMIC_STORAGE_BIT);
        
        //Generate random data
        std::mt19937 rng((unsigned int)GetTimeInMicroseconds());
        GLfloat numDir = 8; //Keep in sync with GLSL shader!!!
        GLfloat rngMax = (GLfloat)rng.max() + 1.f; 
        
        for(unsigned int i=0; i<HBAO_RANDOM_ELEMENTS; ++i)
        {
            GLfloat rand1 = (GLfloat)rng()/rngMax; //random in [0,1)
            GLfloat rand2 = (GLfloat)rng()/rngMax; //random in [0,1)

            //Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
            GLfloat angle = 2.f * M_PI * rand1 / numDir;
            aoData.jitters[i].x = cosf(angle);
            aoData.jitters[i].y = sinf(angle);
            aoData.jitters[i].z = rand2;
            aoData.jitters[i].w = 0;
            
            aoData.float2Offsets[i] = glm::vec4((GLfloat)(i%4) + 0.5f, (GLfloat)(i/4) + 0.5f, 0.0, 0.0);
        }
    }
}

OpenGLCamera::~OpenGLCamera()
{
    glDeleteTextures(1, &renderColorTex);
    glDeleteTextures(1, &renderViewNormalTex);
    glDeleteTextures(1, &renderDepthStencilTex);
    glDeleteTextures(1, &lightMeterTex);
    glDeleteTextures(2, linearDepthTex);
    glDeleteTextures(2, postprocessTex);
    glDeleteTextures(1, &postprocessStencilTex);
    
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteFramebuffers(1, &postprocessFBO);
    glDeleteFramebuffers(1, &lightMeterFBO);
    glDeleteFramebuffers(1, &linearDepthFBO);
    
    if(aoFactor > 0)
    {
        glDeleteTextures(1, &aoResultTex);
        glDeleteTextures(1, &aoBlurTex);
        glDeleteTextures(1, &aoDepthArrayTex);
        glDeleteTextures(1, &aoResultArrayTex);
    
        glDeleteFramebuffers(1, &aoFinalFBO);
        glDeleteFramebuffers(1, &aoDeinterleaveFBO);
        glDeleteFramebuffers(1, &aoCalcFBO);
        
        glDeleteBuffers(1, &aoDataUBO);
    }
}

glm::mat4 OpenGLCamera::GetProjectionMatrix() const
{
    return projection;
}

GLfloat OpenGLCamera::GetFOVX() const
{
    return fovx;
}

GLfloat OpenGLCamera::GetFOVY() const
{
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    return fovx/aspect;
}

GLfloat OpenGLCamera::GetNearClip() const
{
    return near;
}

GLfloat OpenGLCamera::GetFarClip() const
{
    return far;
}

glm::vec3 OpenGLCamera::Ray(GLint x, GLint y)
{
    //Translate point to view space
    x -= originX;
    y -= originY;
    
    //Check if point in view
    if((x < 0) || (x >= viewportWidth) || (y < 0) || (y >= viewportHeight))
        return glm::vec3(0);
        
    glm::vec2 pixPos = glm::vec2((GLfloat)x/(GLfloat)viewportWidth, (GLfloat)(viewportHeight-y)/(GLfloat)viewportHeight);
    pixPos = (pixPos - glm::vec2(0.5f)) * 2.f;
    glm::vec4 deviceRay = glm::vec4(pixPos, 0.f, 1.f);
    glm::vec3 eyeRay = glm::vec3(glm::normalize((glm::inverse(GetProjectionMatrix()) * deviceRay)));
    glm::vec3 worldRay = glm::normalize(glm::inverse(glm::mat3(GetViewMatrix())) * eyeRay);
        
    /*
    //Calculate ray from point
    glm::vec3 _eye = GetEyePosition();
    glm::vec3 _lookingDir = GetLookingDirection();
    glm::vec3 _up = GetUpDirection();
    
    glm::vec3 rayFrom = _eye;
    glm::vec3 rayForward = _lookingDir * far;
    glm::vec3 horizontal = glm::normalize(glm::cross(rayForward, _up));
    glm::vec3 vertical = glm::normalize(glm::cross(horizontal, rayForward));
    
    GLfloat tanFov = tanf(0.5f*fovx);
    horizontal *= 2.f * far * tanFov;
    vertical *= 2.f * far * tanFov;
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    vertical /= aspect;
    
    glm::vec3 rayToCenter = rayFrom + rayForward;
    glm::vec3 dH = horizontal * 1.f/(GLfloat)viewportWidth;
    glm::vec3 dV = vertical * 1.f/(GLfloat)viewportHeight;
    
    glm::vec3 rayTo = rayToCenter - 0.5f * horizontal + 0.5f * vertical;
    rayTo += Scalar(x) * dH;
    rayTo -= Scalar(y) * dV;
    */
    return worldRay;
}
    
void OpenGLCamera::setExposureCompensation(GLfloat ec)
{
    exposureComp = ec;
}

GLuint OpenGLCamera::getPostprocessFBO()
{
    return postprocessFBO;
}
    
GLfloat OpenGLCamera::getExposureCompensation()
{
    return exposureComp;
}

GLuint OpenGLCamera::getColorTexture()
{
    return renderColorTex;
}
    
GLuint OpenGLCamera::getFinalTexture()
{
    return postprocessTex[activePostprocessTexture];
}

bool OpenGLCamera::hasAO()
{
    return aoFactor > 0;
}

bool OpenGLCamera::usingToneMapping()
{
	return toneMapping;
}

bool OpenGLCamera::usingAutoExposure()
{
	return autoExposure;
}

void OpenGLCamera::SetProjection()
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetProjectionMatrix(projection);
}

void OpenGLCamera::SetViewTransform()
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetViewMatrix(GetViewMatrix());
}

void OpenGLCamera::ShowSceneTexture(glm::vec4 rect)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderColorTex);
}

void OpenGLCamera::ShowLinearDepthTexture(glm::vec4 rect, bool frontFace)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, frontFace ? linearDepthTex[0] : linearDepthTex[1]);
}

void OpenGLCamera::ShowViewNormalTexture(glm::vec4 rect)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex);
}

void OpenGLCamera::ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index)
{
    if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoDepthArrayTex, index);
}

void OpenGLCamera::ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index)
{
    if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoResultArrayTex, index);
}

void OpenGLCamera::GenerateLinearDepth(bool front)
{
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, front ? renderDepthStencilTex : postprocessStencilTex);
    OpenGLState::BindFramebuffer(linearDepthFBO);
    GLenum renderBuffs[1];
    renderBuffs[0] = front ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT1;
    glDrawBuffers(1, renderBuffs);    
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    depthLinearizeShader->Use();
    depthLinearizeShader->SetUniform("texLogDepth", TEX_POSTPROCESS1);
    depthLinearizeShader->SetUniform("FC", GetLogDepthConstant());
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

GLuint OpenGLCamera::getLinearDepthTexture(bool frontFace)
{
    return frontFace ? linearDepthTex[0] : linearDepthTex[1];
}

void OpenGLCamera::DrawAO(GLfloat intensity)
{
    if(hasAO())
    {
        //Prepare data and set parameters
        int quarterWidth  = ((viewportWidth+3)/4);
        int quarterHeight = ((viewportHeight+3)/4);
        glm::mat4 proj = GetProjectionMatrix();
        glm::vec4 projInfo(
                            2.0f/proj[0].x,
                            2.0f/proj[1].y,
                            -(1.f-proj[0].z)/proj[0].x,
                            -(1.f+proj[1].z)/proj[1].y
                            );
                          
        glm::vec2 invFullRes(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight);
        glm::vec2 invQuarterRes(1.f/(GLfloat)quarterWidth, 1.f/(GLfloat)quarterHeight);
        GLfloat projScale = (GLfloat)viewportHeight/(tanf(GetFOVY() * 0.5f) * 2.0f);
        GLfloat R = 0.5f;
        
        aoData.projInfo = projInfo;
        aoData.R2 = R * R;
        aoData.NegInvR2 = -1.f/aoData.R2;
        aoData.RadiusToScreen = R * 0.5f * projScale;
        aoData.PowExponent = intensity < 0.f ? 0.f : intensity; //intensity
        aoData.NDotVBias = 0.01f;  //<0,1>
        aoData.AOMultiplier = 1.f/(1.f-aoData.NDotVBias);
        aoData.InvQuarterResolution = invQuarterRes;
        aoData.InvFullResolution = invFullRes;
        
        GLfloat blurSharpness = 40.0f;
       
        GenerateLinearDepth(true);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray(); //Previous function unbinds vertex array
        
        //Deinterleave
        OpenGLState::BindFramebuffer(aoDeinterleaveFBO);
        OpenGLState::Viewport(0, 0, quarterWidth, quarterHeight);

        aoDeinterleaveShader->Use();
        aoDeinterleaveShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex[0]);
        for(int i=0; i<HBAO_RANDOM_ELEMENTS; i+=NUM_MRT)
        {
            aoDeinterleaveShader->SetUniform("info", glm::vec4(float(i % 4) + 0.5f, float(i / 4) + 0.5f, invFullRes.x, invFullRes.y));
            
            for(int layer = 0; layer < NUM_MRT; ++layer)
		    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, aoDepthArrayTex, 0, i+layer);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
            
        //Calculate HBAO
        OpenGLState::BindFramebuffer(aoCalcFBO);
        OpenGLState::Viewport(0, 0, quarterWidth, quarterHeight);
        aoCalcShader->Use();
        aoCalcShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        aoCalcShader->SetUniform("texViewNormal", TEX_POSTPROCESS2);

        OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, renderViewNormalTex);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, aoDataUBO);
        glNamedBufferSubDataEXT(aoDataUBO, 0, sizeof(AOData), &aoData);
        glDrawArrays(GL_TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
               
        //Reinterleave
        OpenGLState::BindFramebuffer(aoFinalFBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
        aoReinterleaveShader->Use();
        aoReinterleaveShader->SetUniform("texResultArray", TEX_POSTPROCESS1);
    
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoResultArrayTex);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    
        //Blur
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        aoBlurShader[0]->Use();
        aoBlurShader[0]->SetUniform("sharpness", blurSharpness);
        aoBlurShader[0]->SetUniform("invResolutionDirection", glm::vec2(1.f/(GLfloat)viewportWidth, 0));
        aoBlurShader[0]->SetUniform("texSource", TEX_POSTPROCESS1);
    
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, aoResultTex);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        
        //Final output to main fbo
        OpenGLState::BindFramebuffer(renderFBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDepthMask(GL_FALSE);
        OpenGLState::DisableDepthTest();
        OpenGLState::EnableBlend();
        //glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_DST_ALPHA);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        aoBlurShader[1]->Use();
        aoBlurShader[1]->SetUniform("sharpness", blurSharpness);
        aoBlurShader[1]->SetUniform("invResolutionDirection", glm::vec2(0, 1.f/(GLfloat)viewportHeight));
        aoBlurShader[1]->SetUniform("texSource", TEX_POSTPROCESS1);
    
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, aoBlurTex);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    
        OpenGLState::EnableDepthTest();
        OpenGLState::DisableBlend();
        glDepthMask(GL_TRUE);
        
        OpenGLState::BindVertexArray(0);
        OpenGLState::UseProgram(0);
        OpenGLState::BindFramebuffer(renderFBO);
    }
}

void OpenGLCamera::DrawSSR()
{
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, renderViewNormalTex);
    OpenGLState::BindTexture(TEX_POSTPROCESS3, GL_TEXTURE_2D, getLinearDepthTexture(true));
    OpenGLState::BindTexture(TEX_POSTPROCESS4, GL_TEXTURE_2D, getLinearDepthTexture(false));
    
    GLfloat sx = viewportWidth/2.f;
    GLfloat sy = viewportHeight/2.f;
    
    glm::mat4 proj = GetProjectionMatrix();
    
    glm::mat4 projPix = glm::transpose(glm::mat4(sx,   0,   0,  sx,
                                                  0, sy,   0,  sy,
                                                  0,   0, 1.f,   0,
                                                  0,   0,   0, 1.f)) * proj;
    
    glm::vec4 projInfo(
                       2.0f/proj[0].x,
                       2.0f/proj[1].y,
                       -(1.f-proj[0].z)/proj[0].x,
                       -(1.f+proj[1].z)/proj[1].y
                       );
    
    ssrShader->Use();
    ssrShader->SetUniform("texColor", TEX_POSTPROCESS1);
    ssrShader->SetUniform("texViewNormal", TEX_POSTPROCESS2);
    ssrShader->SetUniform("texLinearDepth", TEX_POSTPROCESS3);
    ssrShader->SetUniform("texLinearBackfaceDepth", TEX_POSTPROCESS4);
    ssrShader->SetUniform("P", projPix);
    ssrShader->SetUniform("invP", glm::inverse(proj));
    ssrShader->SetUniform("projInfo", projInfo);
    ssrShader->SetUniform("viewportSize", glm::vec2(viewportWidth, viewportHeight));
    ssrShader->SetUniform("invViewportSize", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight));
    ssrShader->SetUniform("near", near);
    ssrShader->SetUniform("far", far);
    ssrShader->SetUniform("maxIterations", 100);
    ssrShader->SetUniform("maxBinarySearchIterations", 10);
    ssrShader->SetUniform("pixelZSize", 1.0f);
    ssrShader->SetUniform("pixelStride", 1.f);
    ssrShader->SetUniform("pixelStrideZCutoff", 50.f);
    ssrShader->SetUniform("maxRayDistance", 500.f);
    ssrShader->SetUniform("screenEdgeFadeStart", 0.9f);
    ssrShader->SetUniform("eyeFadeStart", 0.2f);
    ssrShader->SetUniform("eyeFadeEnd", 0.8f);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS4);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS3);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLCamera::ShowAmbientOcclusion(glm::vec4 rect)
{
    if(hasAO())
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, getAOTexture());
}

GLuint OpenGLCamera::getAOTexture()
{
    return aoBlurTex;
}

GLuint OpenGLCamera::getPostprocessTexture(unsigned int id)
{
    if(id < 2)
        return postprocessTex[id];
    else
        return 0;
}

void OpenGLCamera::DrawLDR(GLuint destinationFBO)
{
	if(usingToneMapping())
	{
		//Bind HDR texture
		OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex);
		
		if(usingAutoExposure())
		{
			//Matrix light metering
			OpenGLState::BindFramebuffer(lightMeterFBO);
			OpenGLState::Viewport(0,0,2,2);
			lightMeterShader->Use();
			lightMeterShader->SetUniform("texHDR", TEX_POSTPROCESS1);
			lightMeterShader->SetUniform("samples", glm::ivec2(64,64));
			((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
			OpenGLState::BindFramebuffer(0);
		
			//Bind exposure texture
			OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, lightMeterTex);
		}
        
        if(antiAliasing)
        {
            OpenGLState::BindFramebuffer(postprocessFBO);
            GLenum renderBuffs[1] = {GL_COLOR_ATTACHMENT1};
            glDrawBuffers(1, renderBuffs);    
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            OpenGLState::Viewport(0, 0, viewportWidth,viewportHeight);
            tonemapShader->Use();
            tonemapShader->SetUniform("texHDR", TEX_POSTPROCESS1);
            tonemapShader->SetUniform("texAverage", TEX_POSTPROCESS2);
            tonemapShader->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp));
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);

            OpenGLState::UnbindTexture(TEX_POSTPROCESS2);   
            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, postprocessTex[1]);

            //Drawing to screen with anti-aliasing
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0,0,viewportWidth,viewportHeight);
            fxaaShader->Use();
            fxaaShader->SetUniform("texSource", TEX_POSTPROCESS1);
            fxaaShader->SetUniform("RCPFrame", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight));
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);
            OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        }
        else
        {
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0, 0, viewportWidth,viewportHeight);
            tonemapShader->Use();
            tonemapShader->SetUniform("texHDR", TEX_POSTPROCESS1);
            tonemapShader->SetUniform("texAverage", TEX_POSTPROCESS2);
            tonemapShader->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp));
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);
            OpenGLState::UnbindTexture(TEX_POSTPROCESS2);   
            OpenGLState::UnbindTexture(TEX_POSTPROCESS1);   
        }

        OpenGLState::UseProgram(0); //Disable shaders
	}
	else
	{
		OpenGLState::BindFramebuffer(destinationFBO);
		OpenGLState::Viewport(0,0,viewportWidth,viewportHeight);
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(renderColorTex);
		OpenGLState::BindFramebuffer(0);
	}
}

///////////////////////// Static /////////////////////////////
void OpenGLCamera::Init()
{
    /////Tonemapping//////
    lightMeterShader = new GLSLShader("lightMeter.frag");
    lightMeterShader->AddUniform("texHDR", ParameterType::INT);
    lightMeterShader->AddUniform("samples", ParameterType::IVEC2);
    
    tonemapShader = new GLSLShader("tonemapping.frag");
    tonemapShader->AddUniform("texHDR", ParameterType::INT);
    tonemapShader->AddUniform("texAverage", ParameterType::INT);
    tonemapShader->AddUniform("exposureComp", ParameterType::FLOAT);
    
    /////Linear depth////
    depthLinearizeShader = new GLSLShader("depthLinearize.frag");
    depthLinearizeShader->AddUniform("texLogDepth", ParameterType::INT);
    depthLinearizeShader->AddUniform("FC", ParameterType::FLOAT);
    
    /////AO//////////////
    if(GLAD_GL_VERSION_4_3)
    {
        aoDeinterleaveShader = new GLSLShader("hbaoDeinterleave.frag");
        aoDeinterleaveShader->AddUniform("info", ParameterType::VEC4);
        aoDeinterleaveShader->AddUniform("texLinearDepth", ParameterType::INT);
        
        aoCalcShader = new GLSLShader("hbaoCalc.frag", "", "saq.geom");
        aoCalcShader->AddUniform("texLinearDepth", ParameterType::INT);
        aoCalcShader->AddUniform("texViewNormal", ParameterType::INT);
        
        aoReinterleaveShader = new GLSLShader("hbaoReinterleave.frag");
        aoReinterleaveShader->AddUniform("texResultArray", ParameterType::INT);
        
        aoBlurShader = new GLSLShader*[2];
        aoBlurShader[0] = new GLSLShader("hbaoBlur.frag");
        aoBlurShader[0]->AddUniform("sharpness", ParameterType::FLOAT);
        aoBlurShader[0]->AddUniform("invResolutionDirection", ParameterType::VEC2);
        aoBlurShader[0]->AddUniform("texSource", ParameterType::INT);
        aoBlurShader[1] = new GLSLShader("hbaoBlur2.frag");
        aoBlurShader[1]->AddUniform("sharpness", ParameterType::FLOAT);
        aoBlurShader[1]->AddUniform("invResolutionDirection", ParameterType::VEC2);
        aoBlurShader[1]->AddUniform("texSource", ParameterType::INT);
    }
    
    //SSR - Screen Space Reflections
    ssrShader = new GLSLShader("ssr.frag");
    ssrShader->AddUniform("texColor", ParameterType::INT);
    ssrShader->AddUniform("texViewNormal", ParameterType::INT);
    ssrShader->AddUniform("texLinearDepth", ParameterType::INT);
    ssrShader->AddUniform("texLinearBackfaceDepth", ParameterType::INT);
    ssrShader->AddUniform("P", ParameterType::MAT4);
    ssrShader->AddUniform("invP", ParameterType::MAT4);
    ssrShader->AddUniform("projInfo", ParameterType::VEC4);
    ssrShader->AddUniform("viewportSize", ParameterType::VEC2);
    ssrShader->AddUniform("invViewportSize", ParameterType::VEC2);
    ssrShader->AddUniform("near", ParameterType::FLOAT);
    ssrShader->AddUniform("far", ParameterType::FLOAT);
    ssrShader->AddUniform("maxIterations", ParameterType::INT);
    ssrShader->AddUniform("maxBinarySearchIterations", ParameterType::INT);
    ssrShader->AddUniform("pixelZSize", ParameterType::FLOAT);
    ssrShader->AddUniform("pixelStride", ParameterType::FLOAT);
    ssrShader->AddUniform("pixelStrideZCutoff", ParameterType::FLOAT);
    ssrShader->AddUniform("maxRayDistance", ParameterType::FLOAT);
    ssrShader->AddUniform("screenEdgeFadeStart", ParameterType::FLOAT);
    ssrShader->AddUniform("eyeFadeStart", ParameterType::FLOAT);
    ssrShader->AddUniform("eyeFadeEnd", ParameterType::FLOAT);

    //FXAA
    fxaaShader = new GLSLShader("fxaa.frag");
    fxaaShader->AddUniform("texSource", ParameterType::INT);
    fxaaShader->AddUniform("RCPFrame", ParameterType::VEC2);
}

void OpenGLCamera::Destroy()
{
    if(lightMeterShader != NULL) delete lightMeterShader;
    if(tonemapShader != NULL) delete tonemapShader;
    if(depthLinearizeShader != NULL) delete depthLinearizeShader;
    if(aoDeinterleaveShader != NULL) delete aoDeinterleaveShader;
    if(aoCalcShader != NULL) delete aoCalcShader;
    if(aoReinterleaveShader != NULL) delete aoReinterleaveShader;
    if(aoBlurShader != NULL)
    {
        if(aoBlurShader[0] != NULL) delete aoBlurShader[0];
        if(aoBlurShader[1] != NULL) delete aoBlurShader[1];
    }
    if(ssrShader != NULL) delete ssrShader;    
    if(fxaaShader != NULL) delete fxaaShader;
}

}
