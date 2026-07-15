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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
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

std::unordered_map<std::string, std::unique_ptr<GLSLShader>> OpenGLCamera::shaders;

OpenGLCamera::OpenGLCamera(GLint x, GLint y, GLint width, GLint height, glm::vec2 range) : OpenGLView(x, y, width, height)
{
    fovx_ = 0.785f;
    near_ = range.x;
    far_ = range.y;
    exposureComp_ = 0.f;
	autoExposure_ = true;
	toneMapping_ = true;
    antiAliasing_ = false;
    aoFactor_ = 0;
    
    if(static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getRenderSettings().ao != RenderQuality::DISABLED)
        aoFactor_ = 1;
    if(static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getRenderSettings().aa != RenderQuality::DISABLED)
        antiAliasing_ = true;
    
    //----Geometry rendering----
    renderColorTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                    GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    renderColorTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                     GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    renderViewNormalTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                         GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);                                                  
    renderDepthStencilTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                           GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL, FilteringMode::NEAREST, false);
    std::vector<FBOTexture> fboTextures;
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderColorTex_[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderViewNormalTex_));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderColorTex_[1]));
    fboTextures.push_back(FBOTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderDepthStencilTex_));
    renderFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);
    OpenGLState::BindFramebuffer(renderFBO_);
    SetRenderBuffers(0, true, false);
    OpenGLState::BindFramebuffer(0);

    //----Postprocessing----
    postprocessTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                    GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    postprocessTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                       GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);
    postprocessStencilTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                           GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL, FilteringMode::NEAREST, false);
    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessTex_[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, postprocessTex_[1]));
    fboTextures.push_back(FBOTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, postprocessStencilTex_));
    postprocessFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);

    quaterPostprocessTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_/2, viewportHeight_/2, 0),
                                                            GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    quaterPostprocessTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_/2, viewportHeight_/2, 0),
                                                            GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, quaterPostprocessTex_[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, quaterPostprocessTex_[1]));                                              
    quaterPostprocessFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);

    //Linear depth
    linearDepthTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                       GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    linearDepthTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                       GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearDepthTex_[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, linearDepthTex_[1]));
    linearDepthFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);

    //---- Tonemapping ----
    histogramBins_ = 256;
    histogramRange_ = glm::vec2(-1.f,11.f);
    GLuint histogram[histogramBins_];
    memset(histogram, 0, histogramBins_ * sizeof(GLuint));
    glGenBuffers(1, &histogramSSBO_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, histogramSSBO_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, histogramBins_ * sizeof(GLuint), histogram, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    GLfloat zero = 1.0f;
    exposureTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(1,1,0), 
                                                 GL_R32F, GL_RED, GL_FLOAT, &zero, FilteringMode::NEAREST, false);
    
    //----HBAO----
    if(aoFactor_ > 0)
    {
        //Deinterleaved results
        GLint swizzle[4] = {GL_RED,GL_GREEN,GL_ZERO,GL_ZERO};
        
        glGenTextures(1, &aoResultTex_);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, aoResultTex_);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, viewportWidth_, viewportHeight_);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
        glGenTextures(1, &aoBlurTex_);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, aoBlurTex_);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, viewportWidth_, viewportHeight_);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        fboTextures.clear();
        fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aoResultTex_));
        fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, aoBlurTex_));
        aoFinalFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);
        
        //Interleaved rendering
        int quarterWidth  = ((viewportWidth_+3)/4);
        int quarterHeight = ((viewportHeight_+3)/4);

        glGenTextures(1, &aoDepthArrayTex_);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D_ARRAY, aoDepthArrayTex_);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenTextures(1, &aoResultArrayTex_);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D_ARRAY, aoResultArrayTex_);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RG16F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        OpenGLState::UnbindTexture(TEX_BASE);
        
        GLenum drawbuffers[NUM_MRT];
        for(int layer = 0; layer < NUM_MRT; ++layer)
            drawbuffers[layer] = GL_COLOR_ATTACHMENT0 + layer;

        glGenFramebuffers(1, &aoDeinterleaveFBO_);
        OpenGLState::BindFramebuffer(aoDeinterleaveFBO_);
        glDrawBuffers(NUM_MRT, drawbuffers);
        
        glGenFramebuffers(1, &aoCalcFBO_);
        OpenGLState::BindFramebuffer(aoCalcFBO_);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoResultArrayTex_, 0);
        OpenGLState::BindFramebuffer(0);
        
        glGenBuffers(1, &aoDataUBO_);
        glNamedBufferStorageEXT(aoDataUBO_, sizeof(AOData), NULL, GL_DYNAMIC_STORAGE_BIT);
        
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
            aoData_.jitters[i].x = cosf(angle);
            aoData_.jitters[i].y = sinf(angle);
            aoData_.jitters[i].z = rand2;
            aoData_.jitters[i].w = 0;
            
            aoData_.float2Offsets[i] = glm::vec4((GLfloat)(i%4) + 0.5f, (GLfloat)(i/4) + 0.5f, 0.0, 0.0);
        }
    }
}

OpenGLCamera::~OpenGLCamera()
{
    glDeleteTextures(2, renderColorTex_);
    glDeleteTextures(1, &renderViewNormalTex_);
    glDeleteTextures(1, &renderDepthStencilTex_);
    glDeleteTextures(1, &exposureTex_);
    glDeleteTextures(2, linearDepthTex_);
    glDeleteTextures(2, postprocessTex_);
    glDeleteTextures(1, &postprocessStencilTex_);
    glDeleteTextures(2, quaterPostprocessTex_);

    glDeleteFramebuffers(1, &renderFBO_);
    glDeleteFramebuffers(1, &postprocessFBO_);
    glDeleteFramebuffers(1, &quaterPostprocessFBO_);
    glDeleteFramebuffers(1, &linearDepthFBO_);
    
    glDeleteBuffers(1, &histogramSSBO_);

    if(aoFactor_ > 0)
    {
        glDeleteTextures(1, &aoResultTex_);
        glDeleteTextures(1, &aoBlurTex_);
        glDeleteTextures(1, &aoDepthArrayTex_);
        glDeleteTextures(1, &aoResultArrayTex_);
    
        glDeleteFramebuffers(1, &aoFinalFBO_);
        glDeleteFramebuffers(1, &aoDeinterleaveFBO_);
        glDeleteFramebuffers(1, &aoCalcFBO_);
        
        glDeleteBuffers(1, &aoDataUBO_);
    }
}

glm::mat4 OpenGLCamera::GetProjectionMatrix() const
{
    return projection_;
}

GLfloat OpenGLCamera::GetFOVX() const
{
    return fovx_;
}

GLfloat OpenGLCamera::GetFOVY() const
{
    GLfloat aspect = (GLfloat)viewportWidth_/(GLfloat)viewportHeight_;
    return fovx_/aspect;
}

GLfloat OpenGLCamera::GetNearClip() const
{
    return near_;
}

GLfloat OpenGLCamera::GetFarClip() const
{
    return far_;
}

glm::vec3 OpenGLCamera::Ray(GLint x, GLint y)
{
    //Translate point to view space
    x -= originX_;
    y -= originY_;
    
    //Check if point in view
    if((x < 0) || (x >= viewportWidth_) || (y < 0) || (y >= viewportHeight_))
        return glm::vec3(0);
        
    glm::vec2 pixPos = glm::vec2((GLfloat)x/(GLfloat)viewportWidth_, (GLfloat)(viewportHeight_-y)/(GLfloat)viewportHeight_);
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
    exposureComp_ = ec;
}

GLuint OpenGLCamera::getPostprocessFBO()
{
    return postprocessFBO_;
}
    
GLuint OpenGLCamera::getQuaterPostprocessFBO()
{
    return quaterPostprocessFBO_;
}

GLfloat OpenGLCamera::getExposureCompensation()
{
    return exposureComp_;
}

GLuint OpenGLCamera::getColorTexture(unsigned int index)
{
    return renderColorTex_[index % 2];
}

GLuint OpenGLCamera::getLinearDepthTexture(bool frontFace)
{
    return frontFace ? linearDepthTex_[0] : linearDepthTex_[1];
}

GLuint OpenGLCamera::getAOTexture()
{
    return aoBlurTex_;
}

GLuint OpenGLCamera::getPostprocessTexture(unsigned int id)
{
    if(id < 2)
        return postprocessTex_[id];
    else
        return 0;
}

GLuint OpenGLCamera::getQuaterPostprocessTexture(unsigned int id)
{
    if(id < 2)
        return quaterPostprocessTex_[id];
    else
        return 0;
}

GLuint OpenGLCamera::getLastActiveColorBuffer()
{
    return lastActiveRenderColorBuffer_;
}

bool OpenGLCamera::hasAO()
{
    return aoFactor_ > 0;
}

bool OpenGLCamera::usingToneMapping()
{
	return toneMapping_;
}

bool OpenGLCamera::usingAutoExposure()
{
	return autoExposure_;
}

void OpenGLCamera::SetProjection()
{
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->SetProjectionMatrix(projection_);
}

void OpenGLCamera::SetViewTransform()
{
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->SetViewMatrix(GetViewMatrix());
}

void OpenGLCamera::SetRenderBuffers(GLuint colorBufferIndex, bool normalBuffer, bool clearBuffers)
{
    GLenum renderBuffs[2];
    renderBuffs[0] = colorBufferIndex == 0 ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT2;
    renderBuffs[1] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(normalBuffer ? 2 : 1, renderBuffs);
    if(clearBuffers) glClear(GL_COLOR_BUFFER_BIT);
    lastActiveRenderColorBuffer_ = colorBufferIndex % 2;
}

void OpenGLCamera::ShowSceneTexture(glm::vec4 rect)
{
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderColorTex_[0]);
}

void OpenGLCamera::ShowLinearDepthTexture(glm::vec4 rect, bool frontFace)
{
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, frontFace ? linearDepthTex_[0] : linearDepthTex_[1]);
}

void OpenGLCamera::ShowViewNormalTexture(glm::vec4 rect)
{
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex_);
}

void OpenGLCamera::ShowDepthStencilTexture(glm::vec4 rect)
{
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderDepthStencilTex_);
}

void OpenGLCamera::ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index)
{
    if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoDepthArrayTex_, index);
}

void OpenGLCamera::ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index)
{
    if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoResultArrayTex_, index);
}

void OpenGLCamera::GenerateLinearDepth(bool front)
{
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, front ? renderDepthStencilTex_ : postprocessStencilTex_);
    OpenGLState::BindFramebuffer(linearDepthFBO_);
    GLenum renderBuffs[1];
    renderBuffs[0] = front ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT1;
    glDrawBuffers(1, renderBuffs);    
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    shaders["depth_linearize"]->Use();
    shaders["depth_linearize"]->SetUniform("texLogDepth", TEX_POSTPROCESS1);
    shaders["depth_linearize"]->SetUniform("FC", GetLogDepthConstant());
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLCamera::DrawAO(GLfloat intensity)
{
    if(hasAO())
    {
        //Prepare data and set parameters
        int quarterWidth  = ((viewportWidth_+3)/4);
        int quarterHeight = ((viewportHeight_+3)/4);
        glm::mat4 proj = GetProjectionMatrix();
        glm::vec4 projInfo(
                            2.0f/proj[0].x,
                            2.0f/proj[1].y,
                            -(1.f-proj[0].z)/proj[0].x,
                            -(1.f+proj[1].z)/proj[1].y
                            );
                          
        glm::vec2 invFullRes(1.f/(GLfloat)viewportWidth_, 1.f/(GLfloat)viewportHeight_);
        glm::vec2 invQuarterRes(1.f/(GLfloat)quarterWidth, 1.f/(GLfloat)quarterHeight);
        GLfloat projScale = (GLfloat)viewportHeight_/(tanf(GetFOVY() * 0.5f) * 2.0f);
        GLfloat R = 0.5f;
        
        aoData_.projInfo = projInfo;
        aoData_.R2 = R * R;
        aoData_.NegInvR2 = -1.f/aoData_.R2;
        aoData_.RadiusToScreen = R * 0.5f * projScale;
        aoData_.PowExponent = intensity < 0.f ? 0.f : intensity; //intensity
        aoData_.NDotVBias = 0.01f;  //<0,1>
        aoData_.AOMultiplier = 1.f/(1.f-aoData_.NDotVBias);
        aoData_.InvQuarterResolution = invQuarterRes;
        aoData_.InvFullResolution = invFullRes;
        
        GLfloat blurSharpness = 40.0f;
       
        GenerateLinearDepth(true);
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray(); //Previous function unbinds vertex array
        
        //Deinterleave
        OpenGLState::BindFramebuffer(aoDeinterleaveFBO_);
        OpenGLState::Viewport(0, 0, quarterWidth, quarterHeight);

        shaders["ao_deinterleave"]->Use();
        shaders["ao_deinterleave"]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex_[0]);
        for(int i=0; i<HBAO_RANDOM_ELEMENTS; i+=NUM_MRT)
        {
            shaders["ao_deinterleave"]->SetUniform("info", glm::vec4(float(i % 4) + 0.5f, float(i / 4) + 0.5f, invFullRes.x, invFullRes.y));
            
            for(int layer = 0; layer < NUM_MRT; ++layer)
		    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, aoDepthArrayTex_, 0, i+layer);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
            
        //Calculate HBAO
        OpenGLState::BindFramebuffer(aoCalcFBO_);
        OpenGLState::Viewport(0, 0, quarterWidth, quarterHeight);
        shaders["ao_calculate"]->Use();
        shaders["ao_calculate"]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        shaders["ao_calculate"]->SetUniform("texViewNormal", TEX_POSTPROCESS2);

        OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, renderViewNormalTex_);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoDepthArrayTex_);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, aoDataUBO_);
        glNamedBufferSubDataEXT(aoDataUBO_, 0, sizeof(AOData), &aoData_);
        glDrawArrays(GL_TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
               
        //Reinterleave
        OpenGLState::BindFramebuffer(aoFinalFBO_);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
        shaders["ao_reinterleave"]->Use();
        shaders["ao_reinterleave"]->SetUniform("texResultArray", TEX_POSTPROCESS1);
    
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoResultArrayTex_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    
        //Blur
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        shaders["ao_blur0"]->Use();
        shaders["ao_blur0"]->SetUniform("sharpness", blurSharpness);
        shaders["ao_blur0"]->SetUniform("invResolutionDirection", glm::vec2(1.f/(GLfloat)viewportWidth_, 0));
        shaders["ao_blur0"]->SetUniform("texSource", TEX_POSTPROCESS1);
    
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, aoResultTex_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        
        //Final output to main fbo
        OpenGLState::BindFramebuffer(renderFBO_);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDepthMask(GL_FALSE);
        OpenGLState::DisableDepthTest();
        OpenGLState::EnableBlend();
        //glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_DST_ALPHA);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        shaders["ao_blur1"]->Use();
        shaders["ao_blur1"]->SetUniform("sharpness", blurSharpness);
        shaders["ao_blur1"]->SetUniform("invResolutionDirection", glm::vec2(0, 1.f/(GLfloat)viewportHeight_));
        shaders["ao_blur1"]->SetUniform("texSource", TEX_POSTPROCESS1);
    
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, aoBlurTex_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    
        OpenGLState::EnableDepthTest();
        OpenGLState::DisableBlend();
        glDepthMask(GL_TRUE);
        
        OpenGLState::BindVertexArray(0);
        OpenGLState::UseProgram(0);
        OpenGLState::BindFramebuffer(renderFBO_);
    }
}

void OpenGLCamera::DrawSSR()
{
    if(shaders["ssr"] == nullptr)
        return;
    
    //Compute SSR
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex_[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, renderViewNormalTex_);
    OpenGLState::BindTexture(TEX_POSTPROCESS3, GL_TEXTURE_2D, getLinearDepthTexture(true));
    OpenGLState::BindTexture(TEX_POSTPROCESS4, GL_TEXTURE_2D, getLinearDepthTexture(false));

    GLfloat sx = viewportWidth_/2.f;
    GLfloat sy = viewportHeight_/2.f;
    
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
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    shaders["ssr"]->Use();
    shaders["ssr"]->SetUniform("texColor", TEX_POSTPROCESS1);
    shaders["ssr"]->SetUniform("texViewNormal", TEX_POSTPROCESS2);
    shaders["ssr"]->SetUniform("texLinearDepth", TEX_POSTPROCESS3);
    shaders["ssr"]->SetUniform("texLinearBackfaceDepth", TEX_POSTPROCESS4);
    shaders["ssr"]->SetUniform("P", projPix);
    shaders["ssr"]->SetUniform("invP", glm::inverse(proj));
    shaders["ssr"]->SetUniform("projInfo", projInfo);
    shaders["ssr"]->SetUniform("viewportSize", glm::vec2(viewportWidth_, viewportHeight_));
    shaders["ssr"]->SetUniform("invViewportSize", glm::vec2(1.f/(GLfloat)viewportWidth_, 1.f/(GLfloat)viewportHeight_));
    shaders["ssr"]->SetUniform("near", near_);
    shaders["ssr"]->SetUniform("far", far_);
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS4);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS3);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);

    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex_[1]);
    SetRenderBuffers(0, false, false);
    OpenGLState::EnableBlend();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shaders["ssr_blur"]->Use();
    shaders["ssr_blur"]->SetUniform("tex", TEX_POSTPROCESS1);
    shaders["ssr_blur"]->SetUniform("invTexSize", glm::vec2(1.f/(GLfloat)viewportWidth_, 1.f/(GLfloat)viewportHeight_));
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::DisableBlend();
    OpenGLState::EnableDepthTest();
}

void OpenGLCamera::GenerateBloom()
{
    //if(shaders["ssr"] == nullptr)
    //    return;
    
    //Initialise bloom
    OpenGLState::BindFramebuffer(quaterPostprocessFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_/2, viewportHeight_/2);
    GLenum renderBuffs[1];
    renderBuffs[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, renderBuffs);
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(renderColorTex_[getLastActiveColorBuffer()]);

    //Blur
    shaders["bloom_blur"]->Use();
    shaders["bloom_blur"]->SetUniform("source", TEX_POSTPROCESS1);

    for(int i=0; i<9; ++i)
    {
        //Horizontal blur
        renderBuffs[0] = GL_COLOR_ATTACHMENT1;
        glDrawBuffers(1, renderBuffs);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, quaterPostprocessTex_[0]);
        shaders["bloom_blur"]->SetUniform("texelOffset", glm::vec2(2.f/(GLfloat)viewportWidth_, 0.f));
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();

        //Vertical blur
        renderBuffs[0] = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, renderBuffs);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, quaterPostprocessTex_[1]);
        shaders["bloom_blur"]->SetUniform("texelOffset", glm::vec2(0.f, 2.f/(GLfloat)viewportHeight_));
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    }
    
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);

    //Restore
    OpenGLState::BindFramebuffer(renderFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
}

void OpenGLCamera::DrawBloom(GLfloat amount)
{
    OpenGLState::DisableDepthTest();
    OpenGLState::EnableBlend();
    glBlendFunc(GL_ONE, GL_ONE);
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(quaterPostprocessTex_[0], glm::vec4(glm::vec3(amount), 0.f));
    OpenGLState::DisableBlend();
    OpenGLState::EnableDepthTest();
}

void OpenGLCamera::ShowAmbientOcclusion(glm::vec4 rect)
{
    if(hasAO())
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, getAOTexture());
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
            
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_HISTOGRAM, histogramSSBO_);
            glBindImageTexture(TEX_POSTPROCESS1, renderColorTex_[lastActiveRenderColorBuffer_], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(TEX_POSTPROCESS2, exposureTex_, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

            //Compute histogram of luminance
            shaders["tonemapping0"]->Use();
            shaders["tonemapping0"]->SetUniform("params", glm::vec2(histogramRange_.x, 1.f/(histogramRange_.y-histogramRange_.x)));
            shaders["tonemapping0"]->SetUniform("texSource", TEX_POSTPROCESS1);
            glDispatchCompute((GLuint)ceilf(viewportWidth_/16.f), (GLuint)ceilf(viewportHeight_/16.f), 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            //Compute exposure
            shaders["tonemapping1"]->Use();
            shaders["tonemapping1"]->SetUniform("texExposure", TEX_POSTPROCESS2);
            shaders["tonemapping1"]->SetUniform("params", glm::vec4(histogramRange_.x, histogramRange_.y-histogramRange_.x, 
                                                                        0.1f, (GLfloat)(viewportWidth_ * viewportHeight_)));    
            glDispatchCompute(1, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

        //Bind color and exposure textures
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex_[lastActiveRenderColorBuffer_]);
		OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, exposureTex_);

        if(antiAliasing_)
        {
            OpenGLState::BindFramebuffer(postprocessFBO_);
            GLenum renderBuffs[1] = {GL_COLOR_ATTACHMENT1};
            glDrawBuffers(1, renderBuffs);    
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
            shaders["tonemapping2"]->Use();
            shaders["tonemapping2"]->SetUniform("texSource", TEX_POSTPROCESS1);
            shaders["tonemapping2"]->SetUniform("texExposure", TEX_POSTPROCESS2);
            shaders["tonemapping2"]->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp_));
            static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);

            OpenGLState::UnbindTexture(TEX_POSTPROCESS2);   
            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, postprocessTex_[1]);

            //Drawing to screen with anti-aliasing
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            shaders["fxaa"]->Use();
            shaders["fxaa"]->SetUniform("texSource", TEX_POSTPROCESS1);
            shaders["fxaa"]->SetUniform("RCPFrame", glm::vec2(1.f/(GLfloat)viewportWidth_, 1.f/(GLfloat)viewportHeight_));
            static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);
            OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        }
        else
        {
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            shaders["tonemapping2"]->Use();
            shaders["tonemapping2"]->SetUniform("texSource", TEX_POSTPROCESS1);
            shaders["tonemapping2"]->SetUniform("texExposure", TEX_POSTPROCESS2);
            shaders["tonemapping2"]->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp_));
            static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::BindFramebuffer(0);
            OpenGLState::UnbindTexture(TEX_POSTPROCESS2);   
            OpenGLState::UnbindTexture(TEX_POSTPROCESS1);   
        }

        OpenGLState::UseProgram(0); //Disable shaders
	}
	else
	{
		OpenGLState::BindFramebuffer(destinationFBO);
		OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
		static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(renderColorTex_[lastActiveRenderColorBuffer_]);
		OpenGLState::BindFramebuffer(0);
	}
}

///////////////////////// Static /////////////////////////////
void OpenGLCamera::Init(const RenderSettings& rSettings)
{
    /////Tonemapping//////
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "lumHistogram.comp"));
    shaders["tonemapping0"] = std::make_unique<GLSLShader>(sources);
    shaders["tonemapping0"]->AddUniform("params", ParameterType::VEC2);
    shaders["tonemapping0"]->AddUniform("texSource", ParameterType::INT);
    shaders["tonemapping0"]->BindShaderStorageBlock("Histogram", SSBO_HISTOGRAM);

    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "autoExposure.comp"));
    shaders["tonemapping1"] = std::make_unique<GLSLShader>(sources);
    shaders["tonemapping1"]->AddUniform("params", ParameterType::VEC4);
    shaders["tonemapping1"]->AddUniform("texExposure", ParameterType::INT);
    shaders["tonemapping1"]->BindShaderStorageBlock("Histogram", SSBO_HISTOGRAM);

    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "saq.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "tonemap.frag"));
    shaders["tonemapping2"] = std::make_unique<GLSLShader>(sources);
    shaders["tonemapping2"]->AddUniform("texSource", ParameterType::INT);
    shaders["tonemapping2"]->AddUniform("texExposure", ParameterType::INT);
    shaders["tonemapping2"]->AddUniform("exposureComp", ParameterType::FLOAT);
    
    /////Linear depth////
    shaders["depth_linearize"] = std::make_unique<GLSLShader>("depthLinearize.frag");
    shaders["depth_linearize"]->AddUniform("texLogDepth", ParameterType::INT);
    shaders["depth_linearize"]->AddUniform("FC", ParameterType::FLOAT);
    
    /////AO//////////////
    if(rSettings.ao != RenderQuality::DISABLED)
    {
        shaders["ao_deinterleave"] = std::make_unique<GLSLShader>("hbaoDeinterleave.frag");
        shaders["ao_deinterleave"]->AddUniform("info", ParameterType::VEC4);
        shaders["ao_deinterleave"]->AddUniform("texLinearDepth", ParameterType::INT);
        
        std::vector<GLSLSource> sources;
        sources.push_back(GLSLSource(GL_VERTEX_SHADER, "saq.vert"));
        sources.push_back(GLSLSource(GL_GEOMETRY_SHADER, "saq.geom"));
        sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "hbaoCalc.frag"));
        shaders["ao_calculate"] = std::make_unique<GLSLShader>(sources);
        shaders["ao_calculate"]->AddUniform("texLinearDepth", ParameterType::INT);
        shaders["ao_calculate"]->AddUniform("texViewNormal", ParameterType::INT);
        
        shaders["ao_reinterleave"] = std::make_unique<GLSLShader>("hbaoReinterleave.frag");
        shaders["ao_reinterleave"]->AddUniform("texResultArray", ParameterType::INT);
        
        shaders["ao_blur0"] = std::make_unique<GLSLShader>("hbaoBlur.frag");
        shaders["ao_blur0"]->AddUniform("sharpness", ParameterType::FLOAT);
        shaders["ao_blur0"]->AddUniform("invResolutionDirection", ParameterType::VEC2);
        shaders["ao_blur0"]->AddUniform("texSource", ParameterType::INT);
        shaders["ao_blur1"] = std::make_unique<GLSLShader>("hbaoBlur2.frag");
        shaders["ao_blur1"]->AddUniform("sharpness", ParameterType::FLOAT);
        shaders["ao_blur1"]->AddUniform("invResolutionDirection", ParameterType::VEC2);
        shaders["ao_blur1"]->AddUniform("texSource", ParameterType::INT);
    }

    //SSR - Screen Space Reflections
    if(rSettings.ssr != RenderQuality::DISABLED)
    {
        shaders["ssr"] = std::make_unique<GLSLShader>("ssr.frag");
        shaders["ssr"]->AddUniform("texColor", ParameterType::INT);
        shaders["ssr"]->AddUniform("texViewNormal", ParameterType::INT);
        shaders["ssr"]->AddUniform("texLinearDepth", ParameterType::INT);
        shaders["ssr"]->AddUniform("texLinearBackfaceDepth", ParameterType::INT);
        shaders["ssr"]->AddUniform("P", ParameterType::MAT4);
        shaders["ssr"]->AddUniform("invP", ParameterType::MAT4);
        shaders["ssr"]->AddUniform("projInfo", ParameterType::VEC4);
        shaders["ssr"]->AddUniform("viewportSize", ParameterType::VEC2);
        shaders["ssr"]->AddUniform("invViewportSize", ParameterType::VEC2);
        shaders["ssr"]->AddUniform("near", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("far", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("maxIterations", ParameterType::INT);
        shaders["ssr"]->AddUniform("maxBinarySearchIterations", ParameterType::INT);
        shaders["ssr"]->AddUniform("pixelZSize", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("pixelStride", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("pixelStrideZCutoff", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("maxRayDistance", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("screenEdgeFadeStart", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("eyeFadeStart", ParameterType::FLOAT);
        shaders["ssr"]->AddUniform("eyeFadeEnd", ParameterType::FLOAT);

        shaders["ssr"]->Use();
        switch(rSettings.ssr)
        {
            case RenderQuality::DISABLED:
            case RenderQuality::LOW:
                shaders["ssr"]->SetUniform("maxIterations", 50);
                shaders["ssr"]->SetUniform("maxBinarySearchIterations", 2);
                shaders["ssr"]->SetUniform("maxRayDistance", 20.f);
                break;

            case RenderQuality::MEDIUM:
                shaders["ssr"]->SetUniform("maxIterations", 100);
                shaders["ssr"]->SetUniform("maxBinarySearchIterations", 5);
                shaders["ssr"]->SetUniform("maxRayDistance", 50.f);
                break;

            case RenderQuality::HIGH:
                shaders["ssr"]->SetUniform("maxIterations", 200);
                shaders["ssr"]->SetUniform("maxBinarySearchIterations", 10);
                shaders["ssr"]->SetUniform("maxRayDistance", 100.f);
                break;
        }
        shaders["ssr"]->SetUniform("pixelStride", 2.f);
        shaders["ssr"]->SetUniform("pixelStrideZCutoff", 3.f);
        shaders["ssr"]->SetUniform("pixelZSize", 0.1f);
        shaders["ssr"]->SetUniform("screenEdgeFadeStart", 0.9f);
        shaders["ssr"]->SetUniform("eyeFadeStart", 0.2f);
        shaders["ssr"]->SetUniform("eyeFadeEnd", 0.8f);
        OpenGLState::UseProgram(0);

        shaders["ssr_blur"] = std::make_unique<GLSLShader>("smallBlur.frag");
        shaders["ssr_blur"]->AddUniform("tex", INT);
        shaders["ssr_blur"]->AddUniform("invTexSize", VEC2);
    }

    //Bloom
    shaders["bloom_blur"] = std::make_unique<GLSLShader>("gaussianBlur.frag", "gaussianBlur.vert");
    shaders["bloom_blur"]->AddUniform("source", INT);
    shaders["bloom_blur"]->AddUniform("texelOffset", VEC2);

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
        shaders["fxaa"] = std::make_unique<GLSLShader>(sources);
        shaders["fxaa"]->AddUniform("texSource", ParameterType::INT);
        shaders["fxaa"]->AddUniform("RCPFrame", ParameterType::VEC2);
    }

    //Real camera
    shaders["flip"] = std::make_unique<GLSLShader>("verticalFlip.frag");
    shaders["flip"]->AddUniform("texSource", ParameterType::INT);
}

void OpenGLCamera::Destroy()
{
    shaders.clear();
}

}