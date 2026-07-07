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
//  OpenGLSSS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 20/06/20.
//  Copyright (c) 2020-2025 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSSS.h"

#include <algorithm>
#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/SSS.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

#define SSS_VRES_FACTOR 0.2f
#define SSS_HRES_FACTOR 100.f

namespace sf
{

OpenGLSSS::OpenGLSSS(glm::vec3 centerPosition, glm::vec3 direction, glm::vec3 forward,
                     GLfloat verticalBeamWidthDeg, GLfloat horizontalBeamWidthDeg, 
                     GLuint numOfBins, GLuint numOfLines, GLfloat verticalTiltDeg, glm::vec2 range, SonarOutputFormat outputFormat)
: OpenGLSonar(centerPosition, direction, forward, glm::uvec2(numOfBins, numOfLines), range, outputFormat)
{
    //SSS specs
    sonar_ = nullptr;
    tilt_ = glm::radians(verticalTiltDeg);
    fov_.x = glm::radians(verticalBeamWidthDeg);
    fov_.y = glm::radians(horizontalBeamWidthDeg);
    nBeamSamples_.x = glm::min((GLuint)ceilf(verticalBeamWidthDeg * (GLfloat)numOfBins/2.f * SSS_VRES_FACTOR), (GLuint)2048);
    nBeamSamples_.y = glm::min((GLuint)ceilf(horizontalBeamWidthDeg * SSS_HRES_FACTOR), (GLuint)2048);
    noise_ = glm::vec2(0.f);
    UpdateTransform();
    
    //Setup matrices
    GLfloat near = range_.x * glm::cos(glm::max(fov_.x/2.f, fov_.y/2.f));
    GLfloat far = range_.y;
    projection_ = glm::perspective(fov_.y, tanf(fov_.x/2.f)/tanf(fov_.y/2.f), near, far);
    GLfloat offsetAngle = M_PI_2 - tilt_;
    views_[0] = glm::rotate(-offsetAngle, glm::vec3(0.f,1.f,0.f));
    views_[1] = glm::rotate(offsetAngle, glm::vec3(0.f,1.f,0.f));

    //Allocate resources
    inputRangeIntensityTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(nBeamSamples_.x, nBeamSamples_.y, 2),
                                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    glGenRenderbuffers(1, &inputDepthRBO_);
    glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO_); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nBeamSamples_.x, nBeamSamples_.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &renderFBO_);
    OpenGLState::BindFramebuffer(renderFBO_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO_);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex_, 0, 0);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, inputRangeIntensityTex_, 0, 1);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");

    //Output shader: sonar range data
    //a. Histograms for vertical beam FOV
    outputTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(nBeamSamples_.x, viewportWidth_/2, 2), 
                                                  GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    //b. Waterfall output (ping-pong) in requested format
    switch (outputFormat)
    {
        case SonarOutputFormat::U8:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                  GL_R8, GL_RED, GL_UNSIGNED_BYTE, NULL, FilteringMode::NEAREST, false);
            outputTex_[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                  GL_R8, GL_RED, GL_UNSIGNED_BYTE, NULL, FilteringMode::NEAREST, false);
            break;

        case SonarOutputFormat::U16:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                          GL_R16, GL_RED, GL_UNSIGNED_SHORT, NULL, FilteringMode::NEAREST, false);
            outputTex_[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                          GL_R16, GL_RED, GL_UNSIGNED_SHORT, NULL, FilteringMode::NEAREST, false);
            break;

        case SonarOutputFormat::U32:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                          GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL, FilteringMode::NEAREST, false);
            outputTex_[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                          GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL, FilteringMode::NEAREST, false);                                              
            break;
        case SonarOutputFormat::F32:
            outputTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                          GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
            outputTex_[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 1), 
                                                          GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
            break;
    }
    pingpong_ = 0;
    //Visualisation shader: sonar data color mapped
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
    
    //Display quad
    GLfloat quadData[4][4];
    quadData[0][0] = -1.f;
    quadData[0][1] = 1.f;
    quadData[0][2] = 0.f;
    quadData[0][3] = 1.f;
    quadData[1][0] = -1.f;
    quadData[1][1] = -1.f;
    quadData[1][2] = 0.f;
    quadData[1][3] = 0.f;
    quadData[2][0] = 1.f;
    quadData[2][1] = 1.f;
    quadData[2][2] = 1.f;
    quadData[2][3] = 1.f;
    quadData[3][0] = 1.f;
    quadData[3][1] = -1.f;
    quadData[3][2] = 1.f;
    quadData[3][3] = 0.f;
    
    glGenVertexArrays(1, &displayVAO_);
    OpenGLState::BindVertexArray(displayVAO_);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &displayVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);

    //Load output shader
    std::string header = "#version 430\n#define N_HALF_BINS " + std::to_string(viewportWidth_/2)
                         + "\n#define N_HORI_BEAM_SAMPLES " + std::to_string(nBeamSamples_.y)
                         + "\n#define N_VERT_BEAM_SAMPLES " + std::to_string(nBeamSamples_.x) + "\n"; ; 
    sonarOutputShader_[0] = new GLSLShader({GLSLSource(GL_COMPUTE_SHADER, "sssOutput.comp", header)});
    sonarOutputShader_[0]->AddUniform("sonarInput", ParameterType::INT);
    sonarOutputShader_[0]->AddUniform("sonarHist", ParameterType::INT);
    sonarOutputShader_[0]->AddUniform("range", ParameterType::VEC3);

    sonarOutputShader_[0]->Use();
    sonarOutputShader_[0]->SetUniform("sonarInput", TEX_POSTPROCESS1);
    sonarOutputShader_[0]->SetUniform("sonarHist", TEX_POSTPROCESS2);
    sonarOutputShader_[0]->SetUniform("range", glm::vec3(range_.x, range_.y, 2*(range_.y-range_.x)/(GLfloat)viewportWidth_));
    OpenGLState::UseProgram(0);

    std::string shaderFilename;
    switch (outputFormat)
    {
        case SonarOutputFormat::U8:
            shaderFilename = "sssLineU8.comp";
            break;
        case SonarOutputFormat::U16:
            shaderFilename = "sssLineU16.comp";
            break;
        case SonarOutputFormat::U32:
            shaderFilename = "sssLineU32.comp";
            break;
        case SonarOutputFormat::F32:
            shaderFilename = "sssLineF32.comp";
            break;
    }
    sonarOutputShader_[1] = new GLSLShader({GLSLSource(GL_COMPUTE_SHADER, shaderFilename, header)});
    sonarOutputShader_[1]->AddUniform("sonarHist", ParameterType::INT);
    sonarOutputShader_[1]->AddUniform("sonarOutput", ParameterType::INT);
    sonarOutputShader_[1]->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarOutputShader_[1]->AddUniform("noiseStddev", ParameterType::VEC2);
    sonarOutputShader_[1]->AddUniform("gain", ParameterType::FLOAT);
    sonarOutputShader_[1]->AddUniform("vfov", ParameterType::FLOAT);
    sonarOutputShader_[1]->AddUniform("tilt", ParameterType::FLOAT);
    sonarOutputShader_[1]->Use();
    sonarOutputShader_[1]->SetUniform("sonarHist", TEX_POSTPROCESS1);
    sonarOutputShader_[1]->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    sonarOutputShader_[1]->SetUniform("gain", gain_);
    sonarOutputShader_[1]->SetUniform("vfov", fov_.x);
    sonarOutputShader_[1]->SetUniform("tilt", tilt_);
    OpenGLState::UseProgram(0);  

    switch (outputFormat)
    {
        case SonarOutputFormat::U8:
            shaderFilename = "sssShiftU8.comp";
            break;
        case SonarOutputFormat::U16:
            shaderFilename = "sssShiftU16.comp";
            break;
        case SonarOutputFormat::U32:
            shaderFilename = "sssShiftU32.comp";
            break;
        case SonarOutputFormat::F32:
            shaderFilename = "sssShiftF32.comp";
            break;
    }
    sonarShiftShader_ = new GLSLShader({GLSLSource(GL_COMPUTE_SHADER, shaderFilename)});
    sonarShiftShader_->AddUniform("sonarOutputIn", ParameterType::INT);
    sonarShiftShader_->AddUniform("sonarOutputOut", ParameterType::INT);
    sonarShiftShader_->Use();
    sonarShiftShader_->SetUniform("sonarOutputIn", TEX_POSTPROCESS1);
    sonarShiftShader_->SetUniform("sonarOutputOut", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);
}

OpenGLSSS::~OpenGLSSS()
{
    delete sonarOutputShader_[0];
    delete sonarOutputShader_[1];
    delete sonarShiftShader_;
    glDeleteTextures(3, outputTex_);
}

void OpenGLSSS::UpdateTransform()
{
    OpenGLSonar::UpdateTransform();

    if(sonar_ == nullptr)
        return;

    //Update settings if necessary
    bool updateProjection = false;
    glm::vec3 rangeGain((GLfloat)sonar_->getRangeMin(), (GLfloat)sonar_->getRangeMax(), (GLfloat)sonar_->getGain());
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
        //settingsUpdated = true;
    }
    if(updateProjection)
    {
        GLfloat near = range_.x/2.f;
        GLfloat far = range_.y;
        projection_[0] = glm::vec4(near/(near*tanf(fov_.x/2.f)), 0.f, 0.f, 0.f);
        projection_[1] = glm::vec4(0.f, near/(near*tanf(fov_.y/2.f)), 0.f, 0.f);
        projection_[2] = glm::vec4(0.f, 0.f, -(far + near)/(far-near), -1.f);
        projection_[3] = glm::vec4(0.f, 0.f, -2.f*far*near/(far-near), 0.f);
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
}

void OpenGLSSS::setNoise(glm::vec2 signalStdDev)
{
    noise_ = signalStdDev;
}

void OpenGLSSS::setSonar(SSS* s)
{
    sonar_ = s;

    glGenBuffers(1, &outputPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
    switch (outputFormat_)
    {
        case SonarOutputFormat::U8:
            glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * sizeof(GLubyte), 0, GL_STREAM_READ);
            break;
        case SonarOutputFormat::U16:
            glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * sizeof(GLushort), 0, GL_STREAM_READ);
            break;
        case SonarOutputFormat::U32:
            glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * sizeof(GLuint), 0, GL_STREAM_READ);
            break;
        case SonarOutputFormat::F32:
            glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * sizeof(GLfloat), 0, GL_STREAM_READ);
            break;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLSSS::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO_);
    OpenGLState::Viewport(0, 0, nBeamSamples_.x, nBeamSamples_.y);
    glDisable(GL_DEPTH_CLAMP);
    sonarInputShader_[1]->Use();
    sonarInputShader_[1]->SetUniform("eyePos", GetEyePosition());
    sonarInputShader_[0]->Use();
    sonarInputShader_[0]->SetUniform("eyePos", GetEyePosition());
    GLSLShader* shader;
    for(size_t i=0; i<2; ++i) //For each of the sonar views
    {
        //Compute matrices
        glm::mat4 VP = GetProjectionMatrix() * views_[i] * GetViewMatrix();
        //Clear color and depth for particular framebuffer layer
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + (GLuint)i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Draw objects
        for(size_t h=0; h<objects.size(); ++h)
        {
            if(objects[h].type != RenderableType::SOLID)
                continue;
            const Object& obj = content->getObject(objects[h].objectId);
            const Look& look = content->getLook(objects[h].lookId);
            glm::mat4 M = objects[h].model;
            Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[h].materialName);
            bool normalMapping = obj.texturable && (look.normalMap > 0);
            shader = normalMapping ? sonarInputShader_[1] : sonarInputShader_[0];
            shader->Use();
            shader->SetUniform("MVP", VP * M);
            shader->SetUniform("M", M);
            shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            shader->SetUniform("restitution", (GLfloat)mat.restitution);
            if(normalMapping)
                OpenGLState::BindTexture(TEX_MAT_NORMAL, GL_TEXTURE_2D, look.normalMap);
            content->DrawObject(objects[h].objectId, objects[h].lookId, objects[h].model);
        }
    }
    glEnable(GL_DEPTH_CLAMP);
    OpenGLState::UnbindTexture(TEX_MAT_NORMAL);
    OpenGLState::BindFramebuffer(0);
    
    //Compute sonar output histogram
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex_, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex_[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
    sonarOutputShader_[0]->Use();
    if(settingsUpdated_)
    {
        sonarOutputShader_[0]->SetUniform("range", glm::vec3(range_.x, range_.y, 2*(range_.y-range_.x)/(GLfloat)viewportWidth_));
        settingsUpdated_ = false;
    }
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBeamSamples_.x/64.f), 2, 1);
    
    //Shift old sonar output
    switch (outputFormat_)
    {
        case SonarOutputFormat::U8:
            glBindImageTexture(TEX_POSTPROCESS1, outputTex_[pingpong_ + 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1-pingpong_ + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
            break;
        case SonarOutputFormat::U16:
            glBindImageTexture(TEX_POSTPROCESS1, outputTex_[pingpong_ + 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1-pingpong_ + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16);
            break;
        case SonarOutputFormat::U32:
            glBindImageTexture(TEX_POSTPROCESS1, outputTex_[pingpong_ + 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1-pingpong_ + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
            break;
        case SonarOutputFormat::F32:
            glBindImageTexture(TEX_POSTPROCESS1, outputTex_[pingpong_ + 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
            glBindImageTexture(TEX_POSTPROCESS2, outputTex_[1-pingpong_ + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
            break;
    }
    sonarShiftShader_->Use();
    glDispatchCompute((GLuint)ceilf(viewportWidth_/16.f), (GLuint)ceilf(viewportHeight_/16.f), 1);
    
    //Postprocess sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex_[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    sonarOutputShader_[1]->Use();
    sonarOutputShader_[1]->SetUniform("noiseSeed", glm::vec3(randDist_(randGen_), randDist_(randGen_), randDist_(randGen_)));
    sonarOutputShader_[1]->SetUniform("noiseStddev", noise_); // Multiplicative, additive (0.01f, 0.02f)
    sonarOutputShader_[1]->SetUniform("gain", gain_);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(viewportWidth_/2.f/64.f), 2, 1);
    
    OpenGLState::BindFramebuffer(displayFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, outputTex_[1-pingpong_ + 1]);
    sonarVisualizeShader_[outputFormat_ == SonarOutputFormat::U32 ? 1 : 0]->Use();
    sonarVisualizeShader_[outputFormat_ == SonarOutputFormat::U32 ? 1 : 0]->SetUniform("texSonarData", TEX_POSTPROCESS2);
    sonarVisualizeShader_[outputFormat_ == SonarOutputFormat::U32 ? 1 : 0]->SetUniform("colorMap", static_cast<GLint>(cMap_));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);

    ++pingpong_;
    if(pingpong_ > 1)
        pingpong_ = 0;
}

void OpenGLSSS::DrawLDR(GLuint destinationFBO, bool updated)
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
        OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
        int windowHeight = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowHeight();
        int windowWidth = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowWidth();
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
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex_[pingpong_+1]);
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
