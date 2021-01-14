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
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLCamera.h"

#include <random>
#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "entities/SolidEntity.h"

namespace sf
{

GLSLShader** OpenGLCamera::tonemappingShaders =  nullptr;
GLSLShader* OpenGLCamera::depthLinearizeShader = nullptr;
GLSLShader* OpenGLCamera::aoDeinterleaveShader = nullptr;
GLSLShader* OpenGLCamera::aoCalcShader = nullptr;
GLSLShader* OpenGLCamera::aoReinterleaveShader = nullptr;
GLSLShader** OpenGLCamera::aoBlurShader = nullptr;
GLSLShader* OpenGLCamera::ssrShader = nullptr;
GLSLShader* OpenGLCamera::fxaaShader = nullptr;
GLSLShader* OpenGLCamera::flipShader = nullptr;
GLSLShader* OpenGLCamera::ssrBlur = nullptr;
GLSLShader* OpenGLCamera::bloomBlur = nullptr;

OpenGLCamera::OpenGLCamera(GLint x, GLint y, GLint width, GLint height, glm::vec2 range) : OpenGLView(x, y, width, height)
{
    fovx = 0.785f;
    near = range.x;
    far = range.y;
    exposureComp = 0.f;
	autoExposure = true;
	toneMapping = true;
    antiAliasing = false;
    aoFactor = 0;
    
    if(((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getRenderSettings().ao != RenderQuality::DISABLED)
        aoFactor = 1;
    if(((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getRenderSettings().aa != RenderQuality::DISABLED)
        antiAliasing = true;
    
    //----Geometry rendering----
    renderColorTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                    GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    renderColorTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                     GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    renderViewNormalTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                         GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);                                                  
    renderDepthStencilTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                           GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL, FilteringMode::NEAREST, false);
    std::vector<FBOTexture> fboTextures;
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderColorTex[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderViewNormalTex));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderColorTex[1]));
    fboTextures.push_back(FBOTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderDepthStencilTex));
    renderFBO = OpenGLContent::GenerateFramebuffer(fboTextures);
    OpenGLState::BindFramebuffer(renderFBO);
    SetRenderBuffers(0, true, false);
    OpenGLState::BindFramebuffer(0);

    //----Postprocessing----
    postprocessTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                    GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    postprocessTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                       GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);
    postprocessStencilTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                           GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL, FilteringMode::NEAREST, false);
    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessTex[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, postprocessTex[1]));
    fboTextures.push_back(FBOTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, postprocessStencilTex));
    postprocessFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

    quaterPostprocessTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth/2, viewportHeight/2, 0),
                                                            GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    quaterPostprocessTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth/2, viewportHeight/2, 0),
                                                            GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, quaterPostprocessTex[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, quaterPostprocessTex[1]));                                              
    quaterPostprocessFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

    //Linear depth
    linearDepthTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                       GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    linearDepthTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                       GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearDepthTex[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, linearDepthTex[1]));
    linearDepthFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

    //---- Tonemapping ----
    histogramBins = 256;
    histogramRange = glm::vec2(-1.f,11.f);
    GLuint histogram[histogramBins];
    memset(histogram, 0, histogramBins * sizeof(GLuint));
    glGenBuffers(1, &histogramSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, histogramSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, histogramBins * sizeof(GLuint), histogram, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    GLfloat zero = 0.f;
    exposureTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(1,1,0), 
                                                 GL_R32F, GL_RED, GL_FLOAT, &zero, FilteringMode::NEAREST, false);
    
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
        
        fboTextures.clear();
        fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aoResultTex));
        fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, aoBlurTex));
        aoFinalFBO = OpenGLContent::GenerateFramebuffer(fboTextures);
        
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
    glDeleteTextures(2, renderColorTex);
    glDeleteTextures(1, &renderViewNormalTex);
    glDeleteTextures(1, &renderDepthStencilTex);
    glDeleteTextures(1, &exposureTex);
    glDeleteTextures(2, linearDepthTex);
    glDeleteTextures(2, postprocessTex);
    glDeleteTextures(1, &postprocessStencilTex);
    glDeleteTextures(2, quaterPostprocessTex);

    glDeleteFramebuffers(1, &renderFBO);
    glDeleteFramebuffers(1, &postprocessFBO);
    glDeleteFramebuffers(1, &quaterPostprocessFBO);
    glDeleteFramebuffers(1, &linearDepthFBO);
    
    glDeleteBuffers(1, &histogramSSBO);

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
    
GLuint OpenGLCamera::getQuaterPostprocessFBO()
{
    return quaterPostprocessFBO;
}

GLfloat OpenGLCamera::getExposureCompensation()
{
    return exposureComp;
}

GLuint OpenGLCamera::getColorTexture(unsigned int index)
{
    return renderColorTex[index % 2];
}

GLuint OpenGLCamera::getLinearDepthTexture(bool frontFace)
{
    return frontFace ? linearDepthTex[0] : linearDepthTex[1];
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

GLuint OpenGLCamera::getQuaterPostprocessTexture(unsigned int id)
{
    if(id < 2)
        return quaterPostprocessTex[id];
    else
        return 0;
}

GLuint OpenGLCamera::getLastActiveColorBuffer()
{
    return lastActiveRenderColorBuffer;
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

void OpenGLCamera::SetRenderBuffers(GLuint colorBufferIndex, bool normalBuffer, bool clearBuffers)
{
    GLenum renderBuffs[2];
    renderBuffs[0] = colorBufferIndex == 0 ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT2;
    renderBuffs[1] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(normalBuffer ? 2 : 1, renderBuffs);
    if(clearBuffers) glClear(GL_COLOR_BUFFER_BIT);
    lastActiveRenderColorBuffer = colorBufferIndex % 2;
}

void OpenGLCamera::ShowSceneTexture(glm::vec4 rect)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderColorTex[0]);
}

void OpenGLCamera::ShowLinearDepthTexture(glm::vec4 rect, bool frontFace)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, frontFace ? linearDepthTex[0] : linearDepthTex[1]);
}

void OpenGLCamera::ShowViewNormalTexture(glm::vec4 rect)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex);
}

void OpenGLCamera::ShowDepthStencilTexture(glm::vec4 rect)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderDepthStencilTex);
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
    if(ssrShader == nullptr)
        return;
    
    //Compute SSR
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex[0]);
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
    
    OpenGLState::BindFramebuffer(getRenderFBO());
    OpenGLState::DisableDepthTest();          
    SetRenderBuffers(1, false, true);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
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
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS4);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS3);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);

    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex[1]);
    SetRenderBuffers(0, false, false);
    OpenGLState::EnableBlend();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ssrBlur->Use();
    ssrBlur->SetUniform("tex", TEX_POSTPROCESS1);
    ssrBlur->SetUniform("invTexSize", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::DisableBlend();
    OpenGLState::EnableDepthTest();
}

void OpenGLCamera::GenerateBloom()
{
    //if(ssrShader == nullptr)
    //    return;
    
    //Initialise bloom
    OpenGLState::BindFramebuffer(quaterPostprocessFBO);
    OpenGLState::Viewport(0, 0, viewportWidth/2, viewportHeight/2);
    GLenum renderBuffs[1];
    renderBuffs[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, renderBuffs);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(renderColorTex[getLastActiveColorBuffer()]);

    //Blur
    bloomBlur->Use();
    bloomBlur->SetUniform("source", TEX_POSTPROCESS1);

    for(int i=0; i<9; ++i)
    {
        //Horizontal blur
        renderBuffs[0] = GL_COLOR_ATTACHMENT1;
        glDrawBuffers(1, renderBuffs);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, quaterPostprocessTex[0]);
        bloomBlur->SetUniform("texelOffset", glm::vec2(2.f/(GLfloat)viewportWidth, 0.f));
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();

        //Vertical blur
        renderBuffs[0] = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, renderBuffs);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, quaterPostprocessTex[1]);
        bloomBlur->SetUniform("texelOffset", glm::vec2(0.f, 2.f/(GLfloat)viewportHeight));
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    }
    
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);

    //Restore
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
}

void OpenGLCamera::DrawBloom(GLfloat amount)
{
    OpenGLState::DisableDepthTest();
    OpenGLState::EnableBlend();
    glBlendFunc(GL_ONE, GL_ONE);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(quaterPostprocessTex[0], glm::vec4(glm::vec3(amount), 0.f));
    OpenGLState::DisableBlend();
    OpenGLState::EnableDepthTest();
}

void OpenGLCamera::ShowAmbientOcclusion(glm::vec4 rect)
{
    if(hasAO())
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, getAOTexture());
}

void OpenGLCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
	if(usingToneMapping())
	{
		if(usingAutoExposure())
		{
            OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
            OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_HISTOGRAM, histogramSSBO);
            glBindImageTexture(TEX_POSTPROCESS1, renderColorTex[lastActiveRenderColorBuffer], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(TEX_POSTPROCESS2, exposureTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

            //Compute histogram of luminance
            tonemappingShaders[0]->Use();
            tonemappingShaders[0]->SetUniform("params", glm::vec2(histogramRange.x, 1.f/(histogramRange.y-histogramRange.x)));
            tonemappingShaders[0]->SetUniform("texSource", TEX_POSTPROCESS1);
            glDispatchCompute((GLuint)ceilf(viewportWidth/16.f), (GLuint)ceilf(viewportHeight/16.f), 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            //Compute exposure
            tonemappingShaders[1]->Use();
            tonemappingShaders[1]->SetUniform("texExposure", TEX_POSTPROCESS2);
            tonemappingShaders[1]->SetUniform("params", glm::vec4(histogramRange.x, histogramRange.y-histogramRange.x, 
                                                                        0.1f, (GLfloat)(viewportWidth * viewportHeight)));    
            glDispatchCompute(1, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            
			//Bind exposure texture
            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex[lastActiveRenderColorBuffer]);
			OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, exposureTex);
		}

        if(antiAliasing)
        {
            OpenGLState::BindFramebuffer(postprocessFBO);
            GLenum renderBuffs[1] = {GL_COLOR_ATTACHMENT1};
            glDrawBuffers(1, renderBuffs);    
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            OpenGLState::Viewport(0, 0, viewportWidth,viewportHeight);
            tonemappingShaders[2]->Use();
            tonemappingShaders[2]->SetUniform("texSource", TEX_POSTPROCESS1);
            tonemappingShaders[2]->SetUniform("texExposure", TEX_POSTPROCESS2);
            tonemappingShaders[2]->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp));
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);

            OpenGLState::UnbindTexture(TEX_POSTPROCESS2);   
            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, postprocessTex[1]);

            //Drawing to screen with anti-aliasing
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0,0,viewportWidth,viewportHeight);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
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
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            tonemappingShaders[2]->Use();
            tonemappingShaders[2]->SetUniform("texSource", TEX_POSTPROCESS1);
            tonemappingShaders[2]->SetUniform("texExposure", TEX_POSTPROCESS2);
            tonemappingShaders[2]->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp));
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
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(renderColorTex[lastActiveRenderColorBuffer]);
		OpenGLState::BindFramebuffer(0);
	}
}

///////////////////////// Static /////////////////////////////
void OpenGLCamera::Init(const RenderSettings& rSettings)
{
    /////Tonemapping//////
    tonemappingShaders = new GLSLShader*[3];
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "lumHistogram.comp"));
    tonemappingShaders[0] = new GLSLShader(sources);
    tonemappingShaders[0]->AddUniform("params", ParameterType::VEC2);
    tonemappingShaders[0]->AddUniform("texSource", ParameterType::INT);
    tonemappingShaders[0]->BindShaderStorageBlock("Histogram", SSBO_HISTOGRAM);

    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "autoExposure.comp"));
    tonemappingShaders[1] = new GLSLShader(sources);
    tonemappingShaders[1]->AddUniform("params", ParameterType::VEC4);
    tonemappingShaders[1]->AddUniform("texExposure", ParameterType::INT);
    tonemappingShaders[1]->BindShaderStorageBlock("Histogram", SSBO_HISTOGRAM);

    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "saq.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "tonemap.frag"));
    tonemappingShaders[2] = new GLSLShader(sources);
    tonemappingShaders[2]->AddUniform("texSource", ParameterType::INT);
    tonemappingShaders[2]->AddUniform("texExposure", ParameterType::INT);
    tonemappingShaders[2]->AddUniform("exposureComp", ParameterType::FLOAT);
    
    /////Linear depth////
    depthLinearizeShader = new GLSLShader("depthLinearize.frag");
    depthLinearizeShader->AddUniform("texLogDepth", ParameterType::INT);
    depthLinearizeShader->AddUniform("FC", ParameterType::FLOAT);
    
    /////AO//////////////
    if(rSettings.ao != RenderQuality::DISABLED)
    {
        aoDeinterleaveShader = new GLSLShader("hbaoDeinterleave.frag");
        aoDeinterleaveShader->AddUniform("info", ParameterType::VEC4);
        aoDeinterleaveShader->AddUniform("texLinearDepth", ParameterType::INT);
        
        std::vector<GLSLSource> sources;
        sources.push_back(GLSLSource(GL_VERTEX_SHADER, "saq.vert"));
        sources.push_back(GLSLSource(GL_GEOMETRY_SHADER, "saq.geom"));
        sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "hbaoCalc.frag"));
        aoCalcShader = new GLSLShader(sources);
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
    if(rSettings.ssr != RenderQuality::DISABLED)
    {
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

        ssrShader->Use();
        switch(rSettings.ssr)
        {
            case RenderQuality::DISABLED:
            case RenderQuality::LOW:
                ssrShader->SetUniform("maxIterations", 50);
                ssrShader->SetUniform("maxBinarySearchIterations", 2);
                ssrShader->SetUniform("maxRayDistance", 20.f);
                break;

            case RenderQuality::MEDIUM:
                ssrShader->SetUniform("maxIterations", 100);
                ssrShader->SetUniform("maxBinarySearchIterations", 5);
                ssrShader->SetUniform("maxRayDistance", 50.f);
                break;

            case RenderQuality::HIGH:
                ssrShader->SetUniform("maxIterations", 200);
                ssrShader->SetUniform("maxBinarySearchIterations", 10);
                ssrShader->SetUniform("maxRayDistance", 100.f);
                break;
        }
        ssrShader->SetUniform("pixelStride", 2.f);
        ssrShader->SetUniform("pixelStrideZCutoff", 3.f);
        ssrShader->SetUniform("pixelZSize", 0.1f);
        ssrShader->SetUniform("screenEdgeFadeStart", 0.9f);
        ssrShader->SetUniform("eyeFadeStart", 0.2f);
        ssrShader->SetUniform("eyeFadeEnd", 0.8f);
        OpenGLState::UseProgram(0);

        ssrBlur = new GLSLShader("smallBlur.frag");
        ssrBlur->AddUniform("tex", INT);
        ssrBlur->AddUniform("invTexSize", VEC2);
    }

    //Bloom
    bloomBlur = new GLSLShader("gaussianBlur.frag", "gaussianBlur.vert");
    bloomBlur->AddUniform("source", INT);
    bloomBlur->AddUniform("texelOffset", VEC2);

    //FXAA
    if(rSettings.aa != RenderQuality::DISABLED)
    {
        std::string header;
        switch(rSettings.aa)
        {
            default:
            case RenderQuality::LOW:
                header = "#version 430\n#define FXAA_QUALITY__PRESET 12\n";
                break;

            case RenderQuality::MEDIUM:
                header = "#version 430\n#define FXAA_QUALITY__PRESET 13\n";
                break;

            case RenderQuality::HIGH:
                header = "#version 430\n#define FXAA_QUALITY__PRESET 23\n";
                break;
        }
        sources.clear();
        sources.push_back(GLSLSource(GL_VERTEX_SHADER, "saq.vert"));
        sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "fxaa.frag", header));
        fxaaShader = new GLSLShader(sources);
        fxaaShader->AddUniform("texSource", ParameterType::INT);
        fxaaShader->AddUniform("RCPFrame", ParameterType::VEC2);
    }

    //Real camera
    flipShader = new GLSLShader("verticalFlip.frag");
    flipShader->AddUniform("texSource", ParameterType::INT);
}

void OpenGLCamera::Destroy()
{
    if(tonemappingShaders != nullptr)
    {
        if(tonemappingShaders[0] != nullptr) delete tonemappingShaders[0];
        if(tonemappingShaders[1] != nullptr) delete tonemappingShaders[1];
        if(tonemappingShaders[2] != nullptr) delete tonemappingShaders[2];
        delete [] tonemappingShaders;
    }
    if(depthLinearizeShader != nullptr) delete depthLinearizeShader;
    if(aoDeinterleaveShader != nullptr) delete aoDeinterleaveShader;
    if(aoCalcShader != nullptr) delete aoCalcShader;
    if(aoReinterleaveShader != nullptr) delete aoReinterleaveShader;
    if(aoBlurShader != nullptr)
    {
        if(aoBlurShader[0] != nullptr) delete aoBlurShader[0];
        if(aoBlurShader[1] != nullptr) delete aoBlurShader[1];
        delete [] aoBlurShader;
    }
    if(ssrShader != nullptr) delete ssrShader;    
    if(fxaaShader != nullptr) delete fxaaShader;
    if(flipShader != nullptr) delete flipShader;
    if(ssrBlur != nullptr) delete ssrBlur;
}

}