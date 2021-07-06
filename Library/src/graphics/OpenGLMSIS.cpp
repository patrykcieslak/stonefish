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
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
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
                       GLuint numOfSteps, GLuint numOfBins, glm::vec2 range_)
: OpenGLSonar(eyePosition, direction, sonarUp, glm::uvec2(numOfBins, numOfBins), range_)
{
    //MSIS specs
    sonar = nullptr;
    nSteps = numOfSteps;
    nBins = numOfBins;
    currentStep = 0;
    beamRotation = glm::mat4(1.f);
    rotationLimits = glm::vec2(-180.f, 180.f);
    nBeamSamples.x = glm::min((GLuint)ceilf(horizontalBeamWidthDeg * (GLfloat)numOfBins * MSIS_RES_FACTOR), (GLuint)2048);
    nBeamSamples.y = glm::min((GLuint)ceilf(verticalBeamWidthDeg * (GLfloat)numOfBins * MSIS_RES_FACTOR), (GLuint)2048);
    noise = glm::vec2(0.f);
    fov.x = glm::radians(horizontalBeamWidthDeg);
    fov.y = glm::radians(verticalBeamWidthDeg);
    UpdateTransform();
    
    //Input shader: range + echo intensity
    //Allocate resources
    inputRangeIntensityTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeamSamples.x, nBeamSamples.y, 1),
                                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    glGenRenderbuffers(1, &inputDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nBeamSamples.x, nBeamSamples.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &renderFBO);
    OpenGLState::BindFramebuffer(renderFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");

    //Setup matrices
    GLfloat near = range.x * glm::cos(glm::max(fov.x/2.f, fov.y/2.f));
    GLfloat far = range.y;
    projection = glm::perspective(fov.y, tanf(fov.x/2.f)/tanf(fov.y/2.f), near, far);
    
    //Output shader: sonar range data
    outputTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeamSamples.y, nBins, 1), 
                                                  GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    outputTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nSteps, nBins, 1), 
                                                  GL_R8, GL_RED, GL_UNSIGNED_BYTE, NULL, FilteringMode::TRILINEAR, false);
    
    //Sonar display fan
    glGenTextures(1, &displayTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, displayTex);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); //RGB image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);
    std::vector<FBOTexture> textures;
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayTex));
    displayFBO = OpenGLContent::GenerateFramebuffer(textures);
    
    //Display fan
    fanDiv = btMin((GLuint)ceil(360), nSteps);
    GLfloat fanData[(fanDiv+1)*2][4];
    GLfloat Rmin = range.x/range.y;
    
    //Flipped vertically to account for OpenGL window coordinates
    for(GLuint i=0; i<fanDiv+1; ++i)
    {
        GLfloat alpha = M_PI - i/(GLfloat)fanDiv * 2 * M_PI;
        //Min range edge
        fanData[i*2][0] = -Rmin*sinf(alpha);
        fanData[i*2][1] = 1.f - Rmin*cosf(alpha) - 1.f;
        fanData[i*2][2] = i/(GLfloat)fanDiv;
        fanData[i*2][3] = 1.f;
        //Max range edge
        fanData[i*2+1][0] = -sinf(alpha);
        fanData[i*2+1][1] = 1.f - cosf(alpha) - 1.f;
        fanData[i*2+1][2] = i/(GLfloat)fanDiv;
        fanData[i*2+1][3] = 0.f;
    }
    
    glGenVertexArrays(1, &displayVAO);
    OpenGLState::BindVertexArray(displayVAO);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &displayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fanData), fanData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);
    
    //Load output shader
    std::string header = "#version 430\n#define N_BINS " + std::to_string(nBins)
                         + "\n#define N_HORI_BEAM_SAMPLES " + std::to_string(nBeamSamples.x) + "\n"; 
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarOutput3.comp", header));
    sonarOutputShader = new GLSLShader(sources);
    sonarOutputShader->AddUniform("sonarInput", ParameterType::INT);
    sonarOutputShader->AddUniform("sonarHist", ParameterType::INT);
    sonarOutputShader->AddUniform("range", ParameterType::VEC3);
    
    sonarOutputShader->Use();
    sonarOutputShader->SetUniform("sonarInput", TEX_POSTPROCESS1);
    sonarOutputShader->SetUniform("sonarHist", TEX_POSTPROCESS2);
    sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
    OpenGLState::UseProgram(0);

    //Load update shader
    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarUpdate.comp"));
    sonarUpdateShader = new GLSLShader(sources);
    sonarUpdateShader->AddUniform("sonarHist", ParameterType::INT);
    sonarUpdateShader->AddUniform("sonarOutput", ParameterType::INT);
    sonarUpdateShader->AddUniform("rotationStep", ParameterType::UINT);
    sonarUpdateShader->AddUniform("gain", ParameterType::FLOAT);
    sonarUpdateShader->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarUpdateShader->AddUniform("noiseStddev", ParameterType::VEC2);
    sonarUpdateShader->Use();
    sonarUpdateShader->SetUniform("sonarHist", TEX_POSTPROCESS1);
    sonarUpdateShader->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);
}

OpenGLMSIS::~OpenGLMSIS()
{
    delete sonarOutputShader;
    delete sonarUpdateShader;
    glDeleteTextures(2, outputTex);
}

void OpenGLMSIS::UpdateTransform()
{
    OpenGLSonar::UpdateTransform();
    
    if(sonar == nullptr)
        return;

    //Update settings if necessary
    bool updateProjection = false;
    glm::vec3 rangeGain((GLfloat)sonar->getRangeMin(), (GLfloat)sonar->getRangeMax(), (GLfloat)sonar->getGain());
    Scalar rotL1, rotL2;
    sonar->getRotationLimits(rotL1, rotL2);
    if(rotationLimits.x != (GLfloat)rotL1 || rotationLimits.y != (GLfloat)rotL2)
    {
        rotationLimits.x = (GLfloat)rotL1;
        rotationLimits.y = (GLfloat)rotL2;
        settingsUpdated = true;
    }

    if(rangeGain.x != range.x)
    {
        range.x = rangeGain.x;
        updateProjection = true;
        settingsUpdated = true;
    }
    if(rangeGain.y != range.y)
    {
        range.y = rangeGain.y;
        updateProjection = true;
        settingsUpdated = true;
    }
    if(rangeGain.z != gain)
    {
        gain = rangeGain.z;
        settingsUpdated = true;
    }
    if(updateProjection)
    {
        GLfloat near = range.x / 2.f;
        GLfloat far = range.y;
        projection[0] = glm::vec4(near/(near*tanf(fov.x/2.f)), 0.f, 0.f, 0.f);
        projection[1] = glm::vec4(0.f, near/(near*tanf(fov.y/2.f)), 0.f, 0.f);
        projection[2] = glm::vec4(0.f, 0.f, -(far + near)/(far-near), -1.f);
        projection[3] = glm::vec4(0.f, 0.f, -2.f*far*near/(far-near), 0.f);

        GLfloat fanData[(fanDiv+1)*2][4];
        GLfloat Rmin = range.x/range.y;
        //Flipped vertically to account for OpenGL window coordinates
        for(GLuint i=0; i<fanDiv+1; ++i)
        {
            GLfloat alpha = M_PI - i/(GLfloat)fanDiv * 2 * M_PI;
            //Min range edge
            fanData[i*2][0] = -Rmin*sinf(alpha);
            fanData[i*2][1] = 1.f - Rmin*cosf(alpha) - 1.f;
            fanData[i*2][2] = i/(GLfloat)fanDiv;
            fanData[i*2][3] = 1.f;
            //Max range edge
            fanData[i*2+1][0] = -sinf(alpha);
            fanData[i*2+1][1] = 1.f - cosf(alpha) - 1.f;
            fanData[i*2+1][2] = i/(GLfloat)fanDiv;
            fanData[i*2+1][3] = 0.f;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fanData), fanData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    //Inform sonar to run callback
    if(newData)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            sonar->NewDataReady(src, 0);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
        src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            sonar->NewDataReady(src, 1);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData = false;
    }

    //Update rotation
    currentStep = sonar->getCurrentRotationStep();
    GLfloat rotAngle = currentStep * (2.f*M_PI/(GLfloat)nSteps);
    beamRotation = glm::rotate(rotAngle, glm::vec3(0.f,1.f,0.f));
}

void OpenGLMSIS::setNoise(glm::vec2 signalStdDev)
{
    noise = signalStdDev;
}

void OpenGLMSIS::setSonar(MSIS* s)
{
    sonar = s;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, nSteps * nBins, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLMSIS::ComputeOutput(std::vector<Renderable>& objects)
{  
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, nBeamSamples.x, nBeamSamples.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_CLAMP);
    sonarInputShader[1]->Use();
    sonarInputShader[1]->SetUniform("eyePos", GetEyePosition());
    sonarInputShader[0]->Use();
    sonarInputShader[0]->SetUniform("eyePos", GetEyePosition());
    GLSLShader* shader;
    
    //Calculate view transform
    glm::mat4 VP = GetProjectionMatrix() * beamRotation * GetViewMatrix();
    //Draw objects
    for(size_t i=0; i<objects.size(); ++i)
    {
        if(objects[i].type != RenderableType::SOLID)
            continue;
        const Object& obj = content->getObject(objects[i].objectId);
        const Look& look = content->getLook(objects[i].lookId);
        glm::mat4 M = objects[i].model;
        Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[i].materialName);
        bool normalMapping = obj.texturable && (look.normalTexture > 0);
        shader = normalMapping ? sonarInputShader[1] : sonarInputShader[0];
        shader->Use();
        shader->SetUniform("MVP", VP * M);
        shader->SetUniform("M", M);
        shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
        shader->SetUniform("restitution", (GLfloat)mat.restitution);
        if(normalMapping)
            OpenGLState::BindTexture(TEX_MAT_NORMAL, GL_TEXTURE_2D, look.normalTexture);
        content->DrawObject(objects[i].objectId, objects[i].lookId, objects[i].model);
    }
    glEnable(GL_DEPTH_CLAMP);
    OpenGLState::UnbindTexture(TEX_MAT_NORMAL);
    OpenGLState::BindFramebuffer(0);

    //Compute sonar output
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    sonarOutputShader->Use();
    if(settingsUpdated)
    {
        sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
        settingsUpdated = false;
        //Clear image
        OpenGLState::BindTexture(TEX_POSTPROCESS3, GL_TEXTURE_2D, outputTex[1]);
        uint8_t zeros[nSteps * nBins];
        memset(zeros, 0, sizeof(zeros));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nSteps, nBins, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)zeros);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS3);
    }
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBeamSamples.y/64.f), 1, 1); 

    //Update sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
    sonarUpdateShader->Use();
    GLuint rotationStep = (GLuint)(currentStep + (GLint)(nSteps/2));
    sonarUpdateShader->SetUniform("rotationStep", rotationStep);
    sonarUpdateShader->SetUniform("gain", gain);
    sonarUpdateShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    sonarUpdateShader->SetUniform("noiseStddev", noise); //Multiplicative, additive (0.02f, 0.04f)
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBins/64.f), 1, 1);
    
    OpenGLState::BindFramebuffer(displayFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex[1]);
    glGenerateMipmap(GL_TEXTURE_2D);
    sonarVisualizeShader->Use();
    sonarVisualizeShader->SetUniform("texSonarData", TEX_POSTPROCESS1);
    sonarVisualizeShader->SetUniform("colormap", static_cast<GLint>(cMap));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (fanDiv+1)*2);
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
    if(sonar != nullptr)
        display = sonar->getDisplayOnScreen(dispX, dispY, dispScale);
    
    //Draw on screen
    if(display)
    {
        OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
        int windowHeight = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowHeight();
        int windowWidth = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowWidth();
        OpenGLState::BindFramebuffer(destinationFBO);    
        OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
        OpenGLState::DisableCullFace();
        content->DrawTexturedQuad(dispX, dispY+viewportHeight*dispScale, viewportWidth*dispScale, -viewportHeight*dispScale, displayTex);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);   
    }
    
    //Copy texture to sonar buffer
    if(sonar != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex[1]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, displayTex);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData = true;
    }
}

}
