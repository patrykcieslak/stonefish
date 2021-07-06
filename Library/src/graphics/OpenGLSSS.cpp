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
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
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
                     GLuint numOfBins, GLuint numOfLines, GLfloat verticalTiltDeg, glm::vec2 range_)
: OpenGLSonar(centerPosition, direction, forward, glm::uvec2(numOfBins, numOfLines), range_)
{
    //SSS specs
    sonar = nullptr;
    tilt = glm::radians(verticalTiltDeg);
    fov.x = glm::radians(verticalBeamWidthDeg);
    fov.y = glm::radians(horizontalBeamWidthDeg);
    nBeamSamples.x = glm::min((GLuint)ceilf(verticalBeamWidthDeg * (GLfloat)numOfBins/2.f * SSS_VRES_FACTOR), (GLuint)2048);
    nBeamSamples.y = glm::min((GLuint)ceilf(horizontalBeamWidthDeg * SSS_HRES_FACTOR), (GLuint)2048);
    noise = glm::vec2(0.f);
    UpdateTransform();
    
    //Setup matrices
    GLfloat near = range.x * glm::cos(glm::max(fov.x/2.f, fov.y/2.f));
    GLfloat far = range.y;
    projection = glm::perspective(fov.y, tanf(fov.x/2.f)/tanf(fov.y/2.f), near, far);
    GLfloat offsetAngle = M_PI_2 - tilt;
    views[0] = glm::rotate(-offsetAngle, glm::vec3(0.f,1.f,0.f));
    views[1] = glm::rotate(offsetAngle, glm::vec3(0.f,1.f,0.f));

    //Allocate resources
    inputRangeIntensityTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(nBeamSamples.x, nBeamSamples.y, 2),
                                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    glGenRenderbuffers(1, &inputDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nBeamSamples.x, nBeamSamples.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &renderFBO);
    OpenGLState::BindFramebuffer(renderFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex, 0, 0);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, inputRangeIntensityTex, 0, 1);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");

    //Output shader: sonar range data
    //a. Histograms for vertical beam FOV
    outputTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(nBeamSamples.x, viewportWidth/2, 2), 
                                                  GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    //b. Waterfall output (ping-pong)
    outputTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 1), 
                                                  GL_R8, GL_RED, GL_UNSIGNED_BYTE, NULL, FilteringMode::NEAREST, false);
    outputTex[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 1), 
                                                  GL_R8, GL_RED, GL_UNSIGNED_BYTE, NULL, FilteringMode::NEAREST, false);
    pingpong = 0;
    //Visualisation shader: sonar data color mapped
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
    
    glGenVertexArrays(1, &displayVAO);
    OpenGLState::BindVertexArray(displayVAO);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &displayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);

    //Load output shader
    std::string header = "#version 430\n#define N_HALF_BINS " + std::to_string(viewportWidth/2)
                         + "\n#define N_HORI_BEAM_SAMPLES " + std::to_string(nBeamSamples.y)
                         + "\n#define N_VERT_BEAM_SAMPLES " + std::to_string(nBeamSamples.x) + "\n"; ; 
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarOutput2.comp", header));
    sonarOutputShader[0] = new GLSLShader(sources);
    sonarOutputShader[0]->AddUniform("sonarInput", ParameterType::INT);
    sonarOutputShader[0]->AddUniform("sonarHist", ParameterType::INT);
    sonarOutputShader[0]->AddUniform("range", ParameterType::VEC3);

    sonarOutputShader[0]->Use();
    sonarOutputShader[0]->SetUniform("sonarInput", TEX_POSTPROCESS1);
    sonarOutputShader[0]->SetUniform("sonarHist", TEX_POSTPROCESS2);
    sonarOutputShader[0]->SetUniform("range", glm::vec3(range.x, range.y, 2*(range.y-range.x)/(GLfloat)viewportWidth));
    OpenGLState::UseProgram(0);

    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarLine.comp", header));
    sonarOutputShader[1] = new GLSLShader(sources);
    sonarOutputShader[1]->AddUniform("sonarHist", ParameterType::INT);
    sonarOutputShader[1]->AddUniform("sonarOutput", ParameterType::INT);
    sonarOutputShader[1]->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarOutputShader[1]->AddUniform("noiseStddev", ParameterType::VEC2);
    sonarOutputShader[1]->AddUniform("gain", ParameterType::FLOAT);
    sonarOutputShader[1]->AddUniform("vfov", ParameterType::FLOAT);
    sonarOutputShader[1]->AddUniform("tilt", ParameterType::FLOAT);
    sonarOutputShader[1]->Use();
    sonarOutputShader[1]->SetUniform("sonarHist", TEX_POSTPROCESS1);
    sonarOutputShader[1]->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    sonarOutputShader[1]->SetUniform("gain", gain);
    sonarOutputShader[1]->SetUniform("vfov", fov.x);
    sonarOutputShader[1]->SetUniform("tilt", tilt);
    OpenGLState::UseProgram(0);  

    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarShift.comp"));
    sonarShiftShader = new GLSLShader(sources);
    sonarShiftShader->AddUniform("sonarOutputIn", ParameterType::INT);
    sonarShiftShader->AddUniform("sonarOutputOut", ParameterType::INT);
    sonarShiftShader->Use();
    sonarShiftShader->SetUniform("sonarOutputIn", TEX_POSTPROCESS1);
    sonarShiftShader->SetUniform("sonarOutputOut", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);
}

OpenGLSSS::~OpenGLSSS()
{
    delete sonarOutputShader[0];
    delete sonarOutputShader[1];
    delete sonarShiftShader;
    glDeleteTextures(3, outputTex);
}

void OpenGLSSS::UpdateTransform()
{
    OpenGLSonar::UpdateTransform();

    if(sonar == nullptr)
        return;

    //Update settings if necessary
    bool updateProjection = false;
    glm::vec3 rangeGain((GLfloat)sonar->getRangeMin(), (GLfloat)sonar->getRangeMax(), (GLfloat)sonar->getGain());
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
        //settingsUpdated = true;
    }
    if(updateProjection)
    {
        GLfloat near = range.x/2.f;
        GLfloat far = range.y;
        projection[0] = glm::vec4(near/(near*tanf(fov.x/2.f)), 0.f, 0.f, 0.f);
        projection[1] = glm::vec4(0.f, near/(near*tanf(fov.y/2.f)), 0.f, 0.f);
        projection[2] = glm::vec4(0.f, 0.f, -(far + near)/(far-near), -1.f);
        projection[3] = glm::vec4(0.f, 0.f, -2.f*far*near/(far-near), 0.f);
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
}

void OpenGLSSS::setNoise(glm::vec2 signalStdDev)
{
    noise = signalStdDev;
}

void OpenGLSSS::setSonar(SSS* s)
{
    sonar = s;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLSSS::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, nBeamSamples.x, nBeamSamples.y);
    glDisable(GL_DEPTH_CLAMP);
    sonarInputShader[1]->Use();
    sonarInputShader[1]->SetUniform("eyePos", GetEyePosition());
    sonarInputShader[0]->Use();
    sonarInputShader[0]->SetUniform("eyePos", GetEyePosition());
    GLSLShader* shader;
    for(size_t i=0; i<2; ++i) //For each of the sonar views
    {
        //Compute matrices
        glm::mat4 VP = GetProjectionMatrix() * views[i] * GetViewMatrix();
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
            bool normalMapping = obj.texturable && (look.normalTexture > 0);
            shader = normalMapping ? sonarInputShader[1] : sonarInputShader[0];
            shader->Use();
            shader->SetUniform("MVP", VP * M);
            shader->SetUniform("M", M);
            shader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            shader->SetUniform("restitution", (GLfloat)mat.restitution);
            if(normalMapping)
                OpenGLState::BindTexture(TEX_MAT_NORMAL, GL_TEXTURE_2D, look.normalTexture);
            content->DrawObject(objects[h].objectId, objects[h].lookId, objects[h].model);
        }
    }
    glEnable(GL_DEPTH_CLAMP);
    OpenGLState::UnbindTexture(TEX_MAT_NORMAL);
    OpenGLState::BindFramebuffer(0);
    
    //Compute sonar output histogram
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
    sonarOutputShader[0]->Use();
    if(settingsUpdated)
    {
        sonarOutputShader[0]->SetUniform("range", glm::vec3(range.x, range.y, 2*(range.y-range.x)/(GLfloat)viewportWidth));
        settingsUpdated = false;
    }
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBeamSamples.x/64.f), 2, 1);
    
    //Shift old sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex[pingpong + 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[1-pingpong + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
    sonarShiftShader->Use();
    glDispatchCompute((GLuint)ceilf(viewportWidth/16.f), (GLuint)ceilf(viewportHeight/16.f), 1);
    
    //Postprocess sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    sonarOutputShader[1]->Use();
    sonarOutputShader[1]->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    sonarOutputShader[1]->SetUniform("noiseStddev", noise); // Multiplicative, additive (0.01f, 0.02f)
    sonarOutputShader[1]->SetUniform("gain", gain);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(viewportWidth/2.f/64.f), 2, 1);
    
    OpenGLState::BindFramebuffer(displayFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, outputTex[1-pingpong + 1]);
    sonarVisualizeShader->Use();
    sonarVisualizeShader->SetUniform("texSonarData", TEX_POSTPROCESS2);
    sonarVisualizeShader->SetUniform("colormap", static_cast<GLint>(cMap));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);

    ++pingpong;
    if(pingpong > 1)
        pingpong = 0;
}

void OpenGLSSS::DrawLDR(GLuint destinationFBO, bool updated)
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
        if(0)
        {    
            content->SetViewportSize(nBeamSamples.x/2, nBeamSamples.y/4);
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0, 0, nBeamSamples.x/2, nBeamSamples.y/4);
            content->DrawTexturedQuad(0.f, 0.f, (GLfloat)nBeamSamples.x/4, (GLfloat)nBeamSamples.y/4, inputRangeIntensityTex, 0, true);
            content->DrawTexturedQuad((GLfloat)nBeamSamples.x/4, 0.f, (GLfloat)nBeamSamples.x/4, (GLfloat)nBeamSamples.y/4, inputRangeIntensityTex, 1, true);
            OpenGLState::BindFramebuffer(0);
        }
        else
        {
            int windowHeight = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowHeight();
            int windowWidth = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowWidth();
            OpenGLState::BindFramebuffer(destinationFBO);    
            OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
            OpenGLState::DisableCullFace();
            content->DrawTexturedQuad(dispX, dispY+viewportHeight*dispScale, viewportWidth*dispScale, -viewportHeight*dispScale, displayTex);
            OpenGLState::EnableCullFace();
            OpenGLState::BindFramebuffer(0);   
        }
    }
    
    //Copy texture to sonar buffer
    if(sonar != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex[pingpong+1]);
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
