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
//  OpenGLMSIS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/07/20.
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLMSIS.h"

#include <algorithm>
#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/MSIS.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

#define MSIS_RES_FACTOR 0.1f

namespace sf
{

OpenGLMSIS::OpenGLMSIS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                       GLfloat horizontalBeamWidthDeg, GLfloat verticalBeamWidthDeg, 
                       GLuint numOfSteps, GLuint numOfBins, glm::vec2 range, SonarOutputFormat outputFormat)
: OpenGLSonar(eyePosition, direction, sonarUp, glm::uvec2(numOfBins, numOfBins), range, outputFormat)
{
    //MSIS specs
    sonar_ = nullptr;
    nSteps_ = numOfSteps;
    nBins_ = numOfBins;
    currentStep_ = 0;
    beamRotation_ = glm::mat4(1.f);
    rotationLimits_ = glm::vec2(-180.f, 180.f);
    nBeamSamples_.x = glm::min((GLuint)ceilf(horizontalBeamWidthDeg * (GLfloat)numOfBins * MSIS_RES_FACTOR), (GLuint)2048);
    nBeamSamples_.y = glm::min((GLuint)ceilf(verticalBeamWidthDeg * (GLfloat)numOfBins * MSIS_RES_FACTOR), (GLuint)2048);
    noise_ = glm::vec2(0.f);
    fov_.x = glm::radians(horizontalBeamWidthDeg);
    fov_.y = glm::radians(verticalBeamWidthDeg);
    UpdateTransform();
    
    //Input shader: range + echo intensity
    //Allocate resources
    inputRangeIntensityTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeamSamples_.x, nBeamSamples_.y, 1),
                                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    glGenRenderbuffers(1, &inputDepthRBO_);
    glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO_); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nBeamSamples_.x, nBeamSamples_.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &renderFBO_);
    OpenGLState::BindFramebuffer(renderFBO_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO_);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex_, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");

    //Setup matrices
    GLfloat near = range_.x * glm::cos(glm::max(fov_.x/2.f, fov_.y/2.f));
    GLfloat far = range_.y;
    projection_ = glm::perspective(fov_.y, tanf(fov_.x/2.f)/tanf(fov_.y/2.f), near, far);
    
    //Output shader: sonar range data
    outputTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeamSamples_.y, nBins_, 1), 
                                                  GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);

    //Output data in requested format
    switch (outputFormat)
    {
        case SonarOutputFormat::U8:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nSteps_, nBins_, 1), 
                                                          GL_R8, GL_RED, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);
            break;

        case SonarOutputFormat::U16:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nSteps_, nBins_, 1), 
                                                          GL_R16, GL_RED, GL_UNSIGNED_SHORT, NULL, FilteringMode::BILINEAR, false);
            break;

        case SonarOutputFormat::U32:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nSteps_, nBins_, 1), 
                                                          GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL, FilteringMode::BILINEAR, false);
            break;
        case SonarOutputFormat::F32:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nSteps_, nBins_, 1), 
                                                          GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
            break;
    }
    
    //Sonar display fan
    glGenTextures(1, &displayTex_);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, displayTex_);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewportWidth_, viewportHeight_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); //RGB image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);
    std::vector<FBOTexture> textures;
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayTex_));
    displayFBO_ = OpenGLContent::GenerateFramebuffer(textures);
    
    //Display fan
    fanDiv_ = btMin((GLuint)ceil(360), nSteps_);
    GLfloat fanData[(fanDiv_+1)*2][4];
    GLfloat Rmin = range_.x/range_.y;
    
    //Flipped vertically to account for OpenGL window coordinates
    for(GLuint i=0; i<fanDiv_+1; ++i)
    {
        GLfloat alpha = M_PI - i/(GLfloat)fanDiv_ * 2 * M_PI;
        //Min range edge
        fanData[i*2][0] = -Rmin*sinf(alpha);
        fanData[i*2][1] = 1.f - Rmin*cosf(alpha) - 1.f;
        fanData[i*2][2] = i/(GLfloat)fanDiv_;
        fanData[i*2][3] = 1.f;
        //Max range edge
        fanData[i*2+1][0] = -sinf(alpha);
        fanData[i*2+1][1] = 1.f - cosf(alpha) - 1.f;
        fanData[i*2+1][2] = i/(GLfloat)fanDiv_;
        fanData[i*2+1][3] = 0.f;
    }
    
    glGenVertexArrays(1, &displayVAO_);
    OpenGLState::BindVertexArray(displayVAO_);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &displayVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fanData), fanData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);
    
    //Load output shader
    std::string header = "#version 430\n#define N_BINS " + std::to_string(nBins_)
                         + "\n#define N_HORI_BEAM_SAMPLES " + std::to_string(nBeamSamples_.x) + "\n"; 
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "msisOutput.comp", header));
    sonarOutputShader_ = std::make_unique<GLSLShader>(sources);
    sonarOutputShader_->AddUniform("sonarInput", ParameterType::INT);
    sonarOutputShader_->AddUniform("sonarHist", ParameterType::INT);
    sonarOutputShader_->AddUniform("range", ParameterType::VEC3);
    
    sonarOutputShader_->Use();
    sonarOutputShader_->SetUniform("sonarInput", TEX_POSTPROCESS1);
    sonarOutputShader_->SetUniform("sonarHist", TEX_POSTPROCESS2);
    sonarOutputShader_->SetUniform("range", glm::vec3(range_.x, range_.y, (range_.y-range_.x)/(GLfloat)nBins_));
    OpenGLState::UseProgram(0);

    //Load update shader
    std::string shaderFilename;
    switch (outputFormat)
    {
        case SonarOutputFormat::U8:
            shaderFilename = "msisUpdateU8.comp";
            break;
        case SonarOutputFormat::U16:
            shaderFilename = "msisUpdateU16.comp";
            break;
        case SonarOutputFormat::U32:
            shaderFilename = "msisUpdateU32.comp";
            break;
        case SonarOutputFormat::F32:
            shaderFilename = "msisUpdateF32.comp";
            break;
    }
    sonarUpdateShader_ = std::make_unique<GLSLShader>(std::vector<GLSLSource>({GLSLSource(GL_COMPUTE_SHADER, shaderFilename)}));
    sonarUpdateShader_->AddUniform("sonarHist", ParameterType::INT);
    sonarUpdateShader_->AddUniform("sonarOutput", ParameterType::INT);
    sonarUpdateShader_->AddUniform("rotationStep", ParameterType::UINT);
    sonarUpdateShader_->AddUniform("gain", ParameterType::FLOAT);
    sonarUpdateShader_->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarUpdateShader_->AddUniform("noiseStddev", ParameterType::VEC2);
    sonarUpdateShader_->Use();
    sonarUpdateShader_->SetUniform("sonarHist", TEX_POSTPROCESS1);
    sonarUpdateShader_->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);
}

OpenGLMSIS::~OpenGLMSIS()
{
    glDeleteTextures(2, outputTex_);
}

void OpenGLMSIS::UpdateTransform()
{
    OpenGLSonar::UpdateTransform();
    
    if(sonar_ == nullptr)
        return;

    //Update settings if necessary
    bool updateProjection = false;
    glm::vec3 rangeGain((GLfloat)sonar_->getRangeMin(), (GLfloat)sonar_->getRangeMax(), (GLfloat)sonar_->getGain());
    Scalar rotL1, rotL2;
    sonar_->getRotationLimits(rotL1, rotL2);
    if(rotationLimits_.x != (GLfloat)rotL1 || rotationLimits_.y != (GLfloat)rotL2)
    {
        rotationLimits_.x = (GLfloat)rotL1;
        rotationLimits_.y = (GLfloat)rotL2;
        settingsUpdated_ = true;
    }

    if(rangeGain.x != range_.x)
    {
        range_.x = rangeGain.x;
        updateProjection = true;
        settingsUpdated_ = true;
    }
    if(rangeGain.y != range_.y)
    {
        range_.y = rangeGain.y;
        updateProjection = true;
        settingsUpdated_ = true;
    }
    if(rangeGain.z != gain_)
    {
        gain_ = rangeGain.z;
        settingsUpdated_ = true;
    }
    if(updateProjection)
    {
        GLfloat near = range_.x / 2.f;
        GLfloat far = range_.y;
        projection_[0] = glm::vec4(near/(near*tanf(fov_.x/2.f)), 0.f, 0.f, 0.f);
        projection_[1] = glm::vec4(0.f, near/(near*tanf(fov_.y/2.f)), 0.f, 0.f);
        projection_[2] = glm::vec4(0.f, 0.f, -(far + near)/(far-near), -1.f);
        projection_[3] = glm::vec4(0.f, 0.f, -2.f*far*near/(far-near), 0.f);

        GLfloat fanData[(fanDiv_+1)*2][4];
        GLfloat Rmin = range_.x/range_.y;
        //Flipped vertically to account for OpenGL window coordinates
        for(GLuint i=0; i<fanDiv_+1; ++i)
        {
            GLfloat alpha = M_PI - i/(GLfloat)fanDiv_ * 2 * M_PI;
            //Min range edge
            fanData[i*2][0] = -Rmin*sinf(alpha);
            fanData[i*2][1] = 1.f - Rmin*cosf(alpha) - 1.f;
            fanData[i*2][2] = i/(GLfloat)fanDiv_;
            fanData[i*2][3] = 1.f;
            //Max range edge
            fanData[i*2+1][0] = -sinf(alpha);
            fanData[i*2+1][1] = 1.f - cosf(alpha) - 1.f;
            fanData[i*2+1][2] = i/(GLfloat)fanDiv_;
            fanData[i*2+1][3] = 0.f;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, displayVBO_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fanData), fanData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    //Inform sonar to run callback
    if(newData_)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
        void* src = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            sonar_->NewDataReady(src, 0);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
        src = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            sonar_->NewDataReady(src, 1);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData_ = false;
    }

    //Update rotation
    currentStep_ = sonar_->getCurrentRotationStep();
    GLfloat rotAngle = currentStep_ * (2.f*M_PI/(GLfloat)nSteps_);
    beamRotation_ = glm::rotate(rotAngle, glm::vec3(0.f,1.f,0.f));
}

void OpenGLMSIS::setNoise(glm::vec2 signalStdDev)
{
    noise_ = signalStdDev;
}

void OpenGLMSIS::setSonar(MSIS* s)
{
    sonar_ = s;

    glGenBuffers(1, &outputPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
    switch (outputFormat_)
    {
        case SonarOutputFormat::U8:
            glBufferData(GL_PIXEL_PACK_BUFFER, nSteps_ * nBins_ * sizeof(GLubyte), 0, GL_STREAM_READ);
            break;
        case SonarOutputFormat::U16:
            glBufferData(GL_PIXEL_PACK_BUFFER, nSteps_ * nBins_ * sizeof(GLushort), 0, GL_STREAM_READ);
            break;
        case SonarOutputFormat::U32:
            glBufferData(GL_PIXEL_PACK_BUFFER, nSteps_ * nBins_ * sizeof(GLuint), 0, GL_STREAM_READ);
            break;
        case SonarOutputFormat::F32:
            glBufferData(GL_PIXEL_PACK_BUFFER, nSteps_ * nBins_ * sizeof(GLfloat), 0, GL_STREAM_READ);
            break;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLMSIS::ComputeOutput(std::vector<Renderable>& objects)
{  
    OpenGLContent* content = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO_);
    OpenGLState::Viewport(0, 0, nBeamSamples_.x, nBeamSamples_.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_CLAMP);
    sonarInputShader_[1]->Use();
    sonarInputShader_[1]->SetUniform("eyePos", GetEyePosition());
    sonarInputShader_[0]->Use();
    sonarInputShader_[0]->SetUniform("eyePos", GetEyePosition());
    GLSLShader* shader;
    
    //Calculate view transform
    glm::mat4 VP = GetProjectionMatrix() * beamRotation_ * GetViewMatrix();
    //Draw objects
    for(size_t i=0; i<objects.size(); ++i)
    {
        if(objects[i].type != RenderableType::SOLID)
            continue;
        const Object& obj = content->getObject(objects[i].objectId);
        const Look& look = content->getLook(objects[i].lookId);
        glm::mat4 M = objects[i].model;
        Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[i].materialName);
        bool normalMapping = obj.texturable && (look.normalMap > 0);
        shader = normalMapping ? sonarInputShader_[1].get() : sonarInputShader_[0].get();
        shader->Use();
        shader->SetUniform("MVP", VP * M);
        shader->SetUniform("M", M);
        shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
        shader->SetUniform("restitution", (GLfloat)mat.restitution);
        if(normalMapping)
            OpenGLState::BindTexture(TEX_MAT_NORMAL, GL_TEXTURE_2D, look.normalMap);
        content->DrawObject(objects[i].objectId, objects[i].lookId, objects[i].model);
    }
    glEnable(GL_DEPTH_CLAMP);
    OpenGLState::UnbindTexture(TEX_MAT_NORMAL);
    OpenGLState::BindFramebuffer(0);

    //Compute sonar output
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex_, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex_[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    sonarOutputShader_->Use();
    if(settingsUpdated_)
    {
        sonarOutputShader_->SetUniform("range", glm::vec3(range_.x, range_.y, (range_.y-range_.x)/(GLfloat)nBins_));
        settingsUpdated_ = false;
        //Clear image
        OpenGLState::BindTexture(TEX_POSTPROCESS3, GL_TEXTURE_2D, outputTex_[1]);
        uint8_t zeros[nSteps_ * nBins_];
        memset(zeros, 0, sizeof(zeros));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nSteps_, nBins_, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)zeros);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS3);
    }
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBeamSamples_.y/64.f), 1, 1); 

    //Update sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex_[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    switch (outputFormat_)
    {
        case SonarOutputFormat::U8:
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
            break;
        case SonarOutputFormat::U16:
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16);
            break;
        case SonarOutputFormat::U32:
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
            break;
        case SonarOutputFormat::F32:
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
            break;
    }
    sonarUpdateShader_->Use();
    GLuint rotationStep = (GLuint)(currentStep_ + (GLint)(nSteps_/2));
    sonarUpdateShader_->SetUniform("rotationStep", rotationStep);
    sonarUpdateShader_->SetUniform("gain", gain_);
    sonarUpdateShader_->SetUniform("noiseSeed", glm::vec3(randDist_(randGen_), randDist_(randGen_), randDist_(randGen_)));
    sonarUpdateShader_->SetUniform("noiseStddev", noise_); //Multiplicative, additive (0.02f, 0.04f)
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBins_/64.f), 1, 1);
    
    OpenGLState::BindFramebuffer(displayFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex_[1]);
    glGenerateMipmap(GL_TEXTURE_2D);
    sonarVisualizeShader_[outputFormat_ == SonarOutputFormat::U32 ? 1 : 0]->Use();
    sonarVisualizeShader_[outputFormat_ == SonarOutputFormat::U32 ? 1 : 0]->SetUniform("texSonarData", TEX_POSTPROCESS1);
    sonarVisualizeShader_[outputFormat_ == SonarOutputFormat::U32 ? 1 : 0]->SetUniform("colorMap", static_cast<GLint>(cMap_));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (fanDiv_+1)*2);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLMSIS::DrawLDR(GLuint destinationFBO, bool updated)
{
    //Check if there is a need to display image on screen
    bool display = true;
    unsigned int dispX, dispY;
    GLfloat dispScale;
    if(sonar_ != nullptr)
        display = sonar_->getDisplayOnScreen(dispX, dispY, dispScale);
    
    //Draw on screen
    if(display)
    {
        OpenGLContent* content = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent();
        int windowHeight = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getWindowHeight();
        int windowWidth = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getWindowWidth();
        OpenGLState::BindFramebuffer(destinationFBO);    
        content->SetViewportSize(windowWidth, windowHeight);
        OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
        OpenGLState::DisableCullFace();
        content->DrawTexturedQuad(dispX, dispY+viewportHeight_*dispScale, viewportWidth_*dispScale, -viewportHeight_*dispScale, displayTex_);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);   
    }
    
    //Copy texture to sonar buffer
    if(sonar_ != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex_[1]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
        switch (outputFormat_)
        {
            case SonarOutputFormat::U8:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
                break;
            case SonarOutputFormat::U16:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
                break;
            case SonarOutputFormat::U32:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
                break;
            case SonarOutputFormat::F32:
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, NULL);
                break;
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, displayTex_);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData_ = true;
    }
}

}
