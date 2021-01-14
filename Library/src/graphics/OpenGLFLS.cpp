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
//  OpenGLFLS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 13/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLFLS.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/FLS.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

#define FLS_MAX_SINGLE_FOV 20.f
#define FLS_VRES_FACTOR 0.1f

namespace sf
{

OpenGLFLS::OpenGLFLS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                     GLfloat horizontalFOVDeg, GLfloat verticalFOVDeg, GLuint numOfBeams, GLuint numOfBins, glm::vec2 range_)
: OpenGLSonar(eyePosition, direction, sonarUp, glm::uvec2(2*numOfBins, numOfBins), range_)
{
    //FLS specs
    sonar = nullptr;
    nBeams = numOfBeams;
    nBins = numOfBins;
    nBeamSamples = glm::min((GLuint)ceilf(verticalFOVDeg * (GLfloat)numOfBins * FLS_VRES_FACTOR), (GLuint)2048);
    noise = glm::vec2(0.f);
    
    fov.x = glm::radians(horizontalFOVDeg);
    fov.y = glm::radians(verticalFOVDeg);
    GLfloat hFactor = sinf(fov.x/2.f);
    viewportWidth = (GLint)ceilf(2.f*hFactor*numOfBins);
    UpdateTransform();

    //Calculate necessary number of camera views
    GLuint nViews = (GLuint)ceilf(horizontalFOVDeg/FLS_MAX_SINGLE_FOV);
    GLuint beams1 = (GLuint)roundf((GLfloat)nBeams/(GLfloat)nViews);
    GLuint beams2 = nBeams - beams1*(nViews-1);
    nViewBeams = glm::max(beams1, beams2);

    //Input shader: range + echo intensity
    //Set number of beams
    for(GLuint i=0; i<nViews-1; ++i) 
    {
        SonarView sv;
        sv.nBeams = beams1;
        views.push_back(sv);
    }
    SonarView sv;
    sv.nBeams = beams2;
    views.push_back(sv);

    //Allocate resources
    inputRangeIntensityTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(nViewBeams, (GLuint)nBeamSamples, nViews),
                                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    glGenRenderbuffers(1, &inputDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nViewBeams, (GLuint)nBeamSamples);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &renderFBO);
    OpenGLState::BindFramebuffer(renderFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex, 0, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");

    //Setup matrices
    GLfloat viewFovCorr = (GLfloat)nViewBeams/(GLfloat)nBeams * fov.x;
    GLfloat near = range.x * glm::cos(glm::max(viewFovCorr/2.f, fov.y/2.f));
    GLfloat far = range.y;
    projection = glm::perspective(fov.y, tanf(viewFovCorr/2.f)/tanf(fov.y/2.f), near, far);
    
    GLfloat viewFovAcc = 0.f;
    for(size_t i=0; i<views.size(); ++i)
    {
        views[i].view = glm::rotate(-fov.x/2.f + viewFovAcc + viewFovCorr/2.f, glm::vec3(0.f,1.f,0.f));   
        GLfloat viewFov = (GLfloat)views[i].nBeams/(GLfloat)nBeams * fov.x;
        viewFovAcc += viewFov;
    }
    
    //Output shader: sonar range data
    outputTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeams, nBins, 1), 
                                                  GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    outputTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeams, nBins, 1), 
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
    
    glGenVertexArrays(1, &displayVAO);
    OpenGLState::BindVertexArray(displayVAO);
    glEnableVertexAttribArray(0);
    
    fanDiv = btMin((GLuint)ceil(horizontalFOVDeg), nBeams);
    GLfloat fanData[(fanDiv+1)*2][4];
    GLfloat Rmin = range.x/range.y;
    
    //Flipped vertically to account for OpenGL window coordinates
    for(GLuint i=0; i<fanDiv+1; ++i)
    {
        GLfloat alpha = fov.x/2.f - i/(GLfloat)fanDiv * fov.x;
        //Min range edge
        fanData[i*2][0] = -Rmin*sinf(alpha)*1.f/hFactor;
        fanData[i*2][1] = (1.f-Rmin*cosf(alpha))*2.f-1.f;
        fanData[i*2][2] = i/(GLfloat)fanDiv;
        fanData[i*2][3] = 1.f;
        //Max range edge
        fanData[i*2+1][0] = -sinf(alpha)*1.f/hFactor;
        fanData[i*2+1][1] = (1.f-cosf(alpha))*2.f-1.f;
        fanData[i*2+1][2] = i/(GLfloat)fanDiv;
        fanData[i*2+1][3] = 0.f;
    }
    
    glGenBuffers(1, &displayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fanData), fanData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);

    //Load output shader
    std::string header = "#version 430\n#define N_BINS " + std::to_string(nBins)
                         + "\n#define N_BEAM_SAMPLES " + std::to_string(nBeamSamples) + "\n"; 
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarOutput.comp", header));
    sonarOutputShader = new GLSLShader(sources);
    sonarOutputShader->AddUniform("sonarInput", ParameterType::INT);
    sonarOutputShader->AddUniform("sonarOutput", ParameterType::INT);
    sonarOutputShader->AddUniform("beams", ParameterType::UVEC2);
    sonarOutputShader->AddUniform("range", ParameterType::VEC3);
    sonarOutputShader->AddUniform("gain", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarOutputShader->AddUniform("noiseStddev", ParameterType::VEC2);
    
    sonarOutputShader->Use();
    sonarOutputShader->SetUniform("sonarInput", TEX_POSTPROCESS1);
    sonarOutputShader->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    sonarOutputShader->SetUniform("beams", glm::uvec2(views.front().nBeams, views.back().nBeams));
    sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
    sonarOutputShader->SetUniform("gain", gain);
    OpenGLState::UseProgram(0);

    //Load postprocess shader
    sources.clear();
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarPostprocess.comp"));
    sonarPostprocessShader = new GLSLShader(sources);
    sonarPostprocessShader->AddUniform("sonarOutput", ParameterType::INT);
    sonarPostprocessShader->AddUniform("sonarPost", ParameterType::INT);
    sonarPostprocessShader->Use();
    sonarPostprocessShader->SetUniform("sonarOutput", TEX_POSTPROCESS1);
    sonarPostprocessShader->SetUniform("sonarPost", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);
}

OpenGLFLS::~OpenGLFLS()
{
    delete sonarOutputShader;
    delete sonarPostprocessShader;
    glDeleteTextures(2, outputTex);
}

void OpenGLFLS::UpdateTransform()
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
        settingsUpdated = true;
    }
    if(updateProjection)
    {
        GLfloat viewFovCorr = (GLfloat)nViewBeams/(GLfloat)nBeams * fov.x;
        GLfloat near = range.x / 2.f;
        GLfloat far = range.y;
        projection[0] = glm::vec4(near/(near*tanf(viewFovCorr/2.f)), 0.f, 0.f, 0.f);
        projection[1] = glm::vec4(0.f, near/(near*tanf(fov.y/2.f)), 0.f, 0.f);
        projection[2] = glm::vec4(0.f, 0.f, -(far + near)/(far-near), -1.f);
        projection[3] = glm::vec4(0.f, 0.f, -2.f*far*near/(far-near), 0.f);

        GLfloat fanData[(fanDiv+1)*2][4];
        GLfloat Rmin = range.x/range.y;
        GLfloat hFactor = sinf(fov.x/2.f);
        //Flipped vertically to account for OpenGL window coordinates
        for(GLuint i=0; i<fanDiv+1; ++i)
        {
            GLfloat alpha = fov.x/2.f - i/(GLfloat)fanDiv * fov.x;
            //Min range edge
            fanData[i*2][0] = -Rmin*sinf(alpha)*1.f/hFactor;
            fanData[i*2][1] = (1.f-Rmin*cosf(alpha))*2.f-1.f;
            fanData[i*2][2] = i/(GLfloat)fanDiv;
            fanData[i*2][3] = 1.f;
            //Max range edge
            fanData[i*2+1][0] = -sinf(alpha)*1.f/hFactor;
            fanData[i*2+1][1] = (1.f-cosf(alpha))*2.f-1.f;
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
}

void OpenGLFLS::setNoise(glm::vec2 signalStdDev)
{
    noise = signalStdDev;
}

void OpenGLFLS::setSonar(FLS* s)
{
    sonar = s;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, nBeams * nBins, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLFLS::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, nViewBeams, nBeamSamples);
    glDisable(GL_DEPTH_CLAMP);
    sonarInputShader[1]->Use();
    sonarInputShader[1]->SetUniform("eyePos", GetEyePosition());
    sonarInputShader[0]->Use();
    sonarInputShader[0]->SetUniform("eyePos", GetEyePosition());
    GLSLShader* shader;
    for(size_t i=0; i<views.size(); ++i) //For each of the sonar views
    {
        //Clear color and depth for particular framebuffer layer
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex, 0, i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Calculate view transform
        glm::mat4 VP = GetProjectionMatrix() * views[i].view * GetViewMatrix();
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

    //Compute sonar output
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    sonarOutputShader->Use();
    sonarOutputShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    sonarOutputShader->SetUniform("noiseStddev", noise); //Multiplicative, additive (0.025f, 0.035f)
    if(settingsUpdated)
    {
        sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
        sonarOutputShader->SetUniform("gain", gain);
        settingsUpdated = false;
    }
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nViewBeams/64.f), (GLuint)views.size(), 1); 

    //Postprocess sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
    sonarPostprocessShader->Use();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBeams/16.f), (GLuint)ceilf(nBins/16.f), 1);
    
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

void OpenGLFLS::DrawLDR(GLuint destinationFBO, bool updated)
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
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0,0,nBeams,nBeamSamples);
            GLuint offset = 0;
            for(size_t i=0; i<views.size(); ++i)
            {
                content->DrawTexturedQuad((GLfloat)offset, 0.f, (GLfloat)nViewBeams, (GLfloat)nBeamSamples, 
                                                                                    inputRangeIntensityTex, i, true);
                offset += views[i].nBeams;
            }
            OpenGLState::BindFramebuffer(0);
        }
        else
        {
            int windowHeight = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowHeight();
            OpenGLState::BindFramebuffer(destinationFBO);    
            OpenGLState::Viewport(dispX, windowHeight-viewportHeight*dispScale-dispY, viewportWidth*dispScale, viewportHeight*dispScale);
            content->DrawTexturedSAQ(displayTex);
            OpenGLState::BindFramebuffer(0);   
        }
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
