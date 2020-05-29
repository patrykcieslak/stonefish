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
//  OpenGLFLS2.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 13/02/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLFLS2.h"

#include <algorithm>
#include "core/Console.h"
#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/FLS.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

GLSLShader* OpenGLFLS2::sonarInputShader = NULL;
GLSLShader* OpenGLFLS2::sonarVisualizeShader = NULL;

OpenGLFLS2::OpenGLFLS2(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                  GLint originX, GLint originY, GLfloat horizontalFOVDeg, GLfloat verticalFOVDeg, GLuint numOfBeams, 
                  GLint beamHPix, GLint beamVPix, GLfloat minRange, GLfloat maxRange, GLuint numOfBins)
 : OpenGLView(originX, originY, 2*numOfBins, numOfBins), randDist(0.f, 1.f)
{
    _needsUpdate = false;
    update = false;
    sonar = NULL;
    range.x = minRange;
    range.y = maxRange;
    nBeams = numOfBeams;
    nBins = numOfBins;
    nBeamSamples = 1000; //beamVPix;
    cMap = ColorMap::COLORMAP_HOT;
    render2FBO = 0;
    inputRangeIntensity2Tex = 0;
    
    SetupSonar(eyePosition, direction, sonarUp);
    UpdateTransform();
    
    fov.x = horizontalFOVDeg/180.f*M_PI;
    fov.y = verticalFOVDeg/180.f*M_PI;
    GLfloat hFactor = sinf(fov.x/2.f);
    viewportWidth = (GLint)ceilf(2.f*hFactor*numOfBins);

    //viewportWidth = nBeams;
    //viewportHeight = nBeamSamples;

    //Calculate necessary number of camera views
    GLuint nViews = (GLuint)ceilf(horizontalFOVDeg/FLS_MAX_SINGLE_FOV);
    GLuint beams1 = (GLuint)roundf((GLfloat)nBeams/(GLfloat)nViews);
    GLuint beams2 = nBeams - beams1*(nViews-1);
    
    //Input shader: range + echo intensity
    if(nViews == 1)
    {
        SonarView sv;
        sv.nBeams = nBeams;        
        views.push_back(sv);

        inputRangeIntensityTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeams, (GLuint)nBeamSamples, 1), 
                                                                GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
        glGenRenderbuffers(1, &inputDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nBeams, (GLuint)nBeamSamples);  
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glGenFramebuffers(1, &renderFBO);
        OpenGLState::BindFramebuffer(renderFBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
            cError("Sonar input FBO initialization failed!");
    }
    else
    {
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

        if(beams1 == beams2) //One framebuffer and one texture needed!
        {
            inputRangeIntensityTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(beams1, (GLuint)nBeamSamples, nViews), 
                                                                    GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
            glGenRenderbuffers(1, &inputDepthRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, beams1, (GLuint)nBeamSamples);  
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glGenFramebuffers(1, &renderFBO);
            OpenGLState::BindFramebuffer(renderFBO);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
            for(GLuint i=0; i<nViews; ++i)
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, inputRangeIntensityTex, 0, i);
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(status != GL_FRAMEBUFFER_COMPLETE)
                cError("Sonar input FBO initialization failed!");
        }
        else //Two framebuffers and two textures needed!
        {
            inputRangeIntensityTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(beams1, (GLuint)nBeamSamples, nViews-1), 
                                                                    GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
            glGenRenderbuffers(1, &inputDepthRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, beams1, (GLuint)nBeamSamples);  
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glGenFramebuffers(1, &renderFBO);
            OpenGLState::BindFramebuffer(renderFBO);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
            for(GLuint i=0; i<nViews-1; ++i)
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, inputRangeIntensityTex, 0, i);
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(status != GL_FRAMEBUFFER_COMPLETE)
                cError("Sonar input FBO initialization failed!");

            inputRangeIntensity2Tex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(beams2, (GLuint)nBeamSamples, 1), 
                                                                     GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
            glGenRenderbuffers(1, &inputDepth2RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, inputDepth2RBO); 
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, beams2, (GLuint)nBeamSamples);  
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glGenFramebuffers(1, &render2FBO);
            OpenGLState::BindFramebuffer(render2FBO);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepth2RBO);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensity2Tex, 0);
            status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(status != GL_FRAMEBUFFER_COMPLETE)
                cError("Sonar input FBO initialization failed!");
        }
    }

    //Setup matrices
    glm::mat4 proj;
    proj[1] = glm::vec4(0.f, range.x/(range.x*tanf(fov.y/2.f)), 0.f, 0.f);
    proj[2] = glm::vec4(0.f, 0.f, -(range.y + range.x)/(range.y-range.x), -1.f);
    proj[3] = glm::vec4(0.f, 0.f, -2.f*range.y*range.x/(range.y-range.x), 0.f);
    GLfloat viewFovAcc = 0.f;
    for(size_t i=0; i<views.size(); ++i)
    {
        GLfloat viewFov = (GLfloat)views[i].nBeams/(GLfloat)nBeams * fov.x;
        proj[0] = glm::vec4(range.x/(range.x*tanf(viewFov/2.f)), 0.f, 0.f, 0.f);
        views[i].projection = proj;
        views[i].view = glm::rotate(-fov.x/2.f + viewFovAcc + viewFov/2.f, glm::vec3(0.f,1.f,0.f));
        viewFovAcc += viewFov;
    }
    
    //Output shader: sonar range data
    outputTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeams, nBins, 1), 
                                               GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    glGenFramebuffers(1, &outputFBO);
    OpenGLState::BindFramebuffer(outputFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTex, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar output FBO initialization failed!");
        
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
    
    glGenFramebuffers(1, &displayFBO);
    OpenGLState::BindFramebuffer(displayFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, displayTex, 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar display FBO initialization failed!");
    OpenGLState::BindFramebuffer(0);
    
    glGenVertexArrays(1, &fanVAO);
    OpenGLState::BindVertexArray(fanVAO);
    glEnableVertexAttribArray(0);
    
    fanDiv = std::min((GLuint)ceil(horizontalFOVDeg), nBeams);
    GLfloat fanData[(fanDiv+1)*2][4];
    GLfloat Rmin = range.x/range.y;
    
    for(GLuint i=0; i<fanDiv+1; ++i)
    {
        GLfloat alpha = fov.x/2.f - i/(GLfloat)fanDiv * fov.x;
        fanData[i*2][0] = Rmin*sinf(alpha)*1.f/hFactor;
        fanData[i*2][1] = Rmin*cosf(alpha)*2.f-1.f;
        fanData[i*2][2] = 1.f-i/(GLfloat)fanDiv;
        fanData[i*2][3] = 0.f;
        fanData[i*2+1][0] = sinf(alpha)*1.f/hFactor;
        fanData[i*2+1][1] = cosf(alpha)*2.f-1.f;
        fanData[i*2+1][2] = 1.f-i/(GLfloat)fanDiv;
        fanData[i*2+1][3] = 1.f;
    }
    
    glGenBuffers(1, &fanBuf);
    glBindBuffer(GL_ARRAY_BUFFER, fanBuf);
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
    sonarOutputShader->AddUniform("beamOffset", ParameterType::UINT);
    sonarOutputShader->AddUniform("range", ParameterType::VEC3);
    sonarOutputShader->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarOutputShader->AddUniform("noiseStddev", ParameterType::VEC2);
}

OpenGLFLS2::~OpenGLFLS2()
{
    delete sonarOutputShader;
    glDeleteTextures(1, &inputRangeIntensityTex);
    glDeleteRenderbuffers(1, &inputDepthRBO);
    glDeleteFramebuffers(1, &renderFBO);
    if(inputRangeIntensity2Tex != 0) 
    {
        glDeleteTextures(1, &inputRangeIntensity2Tex);
        glDeleteRenderbuffers(1, &inputDepth2RBO);
        glDeleteFramebuffers(1, &render2FBO);
    }
    glDeleteTextures(1, &outputTex);
    glDeleteFramebuffers(1, &outputFBO);
    glDeleteTextures(1, &displayTex);
    glDeleteFramebuffers(1, &displayFBO);
    glDeleteBuffers(1, &fanBuf);
    glDeleteVertexArrays(1, &fanVAO);
}

void OpenGLFLS2::SetupSonar(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempEye = _eye;
    tempUp = _up;
}

void OpenGLFLS2::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupSonar();
}

void OpenGLFLS2::SetupSonar()
{
    sonarTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLFLS2::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLFLS2::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLFLS2::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLFLS2::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLFLS2::GetViewMatrix() const
{
    return sonarTransform;
}

GLfloat OpenGLFLS2::GetFarClip() const
{
    return range.y;
}

void OpenGLFLS2::Update()
{
    _needsUpdate = true;
}

bool OpenGLFLS2::needsUpdate()
{
    update = _needsUpdate;
    _needsUpdate = false;
    return update && enabled;
}

void OpenGLFLS2::setColorMap(ColorMap cm)
{
    cMap = cm;
}

void OpenGLFLS2::setSonar(FLS* s)
{
    sonar = s;
}

ViewType OpenGLFLS2::getType()
{
    return SONAR;
}

void OpenGLFLS2::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    
    if(views.size() == 1)
    {
        //Generate sonar input
        OpenGLState::BindFramebuffer(renderFBO);
        OpenGLState::Viewport(0, 0, nBeams, nBeamSamples);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sonarInputShader->Use();
        sonarInputShader->SetUniform("eyePos", GetEyePosition());
        glm::mat4 VP = views[0].projection * GetViewMatrix();
    
        for(size_t i=0; i<objects.size(); ++i)
        {
            if(objects[i].type != RenderableType::SOLID)
                continue;
            glm::mat4 M = objects[i].model;
            Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[i].materialName);
            sonarInputShader->SetUniform("MVP", VP * M);
            sonarInputShader->SetUniform("M", M);
            sonarInputShader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            sonarInputShader->SetUniform("restitution", (GLfloat)mat.restitution);
            content->DrawObject(objects[i].objectId, objects[i].lookId, objects[i].model);
        }
        OpenGLState::UseProgram(0);
        OpenGLState::BindFramebuffer(0);
        
        //Compute sonar output
        glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
        glBindImageTexture(TEX_POSTPROCESS2, outputTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        sonarOutputShader->Use();
        sonarOutputShader->SetUniform("sonarInput", TEX_POSTPROCESS1);
        sonarOutputShader->SetUniform("sonarOutput", TEX_POSTPROCESS2);
        sonarOutputShader->SetUniform("beamOffset", (GLuint)0);
        sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
        sonarOutputShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), 
                                                             randDist(randGen), 
                                                             randDist(randGen)));
        sonarOutputShader->SetUniform("noiseStddev", glm::vec2(0.1f, 0.05f));
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute((GLuint)ceilf(nBeams/16.f), 1, 1);
        OpenGLState::UseProgram(0);
    }
    else
    {
        //Generate sonar input
        bool allEqual = views.front().nBeams == views.back().nBeams; //Are all views equal size?
        OpenGLState::BindFramebuffer(renderFBO);
        OpenGLState::Viewport(0, 0, views.front().nBeams, nBeamSamples);
        sonarInputShader->Use();
        sonarInputShader->SetUniform("eyePos", GetEyePosition());

        for(size_t i=0; i<views.size()-(allEqual ? 0 : 1); ++i) //For each of the sonar views
        {
            //Clear color and depth for particular framebuffer layer
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + (GLuint)i);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //Calculate view transform
            glm::mat4 VP = views[i].projection * views[i].view * GetViewMatrix();
                
            for(size_t h=0; h<objects.size(); ++h)
            {
                if(objects[h].type != RenderableType::SOLID)
                    continue;
                glm::mat4 M = objects[h].model;
                Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[h].materialName);
                sonarInputShader->SetUniform("MVP", VP * M);
                sonarInputShader->SetUniform("M", M);
                sonarInputShader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
                sonarInputShader->SetUniform("restitution", (GLfloat)mat.restitution);
                content->DrawObject(objects[h].objectId, objects[h].lookId, objects[h].model);
            }
        }

        if(!allEqual) //Last view has different size
        {
            OpenGLState::BindFramebuffer(render2FBO);
            OpenGLState::Viewport(0, 0, views.back().nBeams, nBeamSamples);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 VP = views.back().projection * views.back().view * GetViewMatrix();
    
            for(size_t i=0; i<objects.size(); ++i)
            {
                if(objects[i].type != RenderableType::SOLID)
                    continue;
                glm::mat4 M = objects[i].model;
                Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[i].materialName);
                sonarInputShader->SetUniform("MVP", VP * M);
                sonarInputShader->SetUniform("M", M);
                sonarInputShader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
                sonarInputShader->SetUniform("restitution", (GLfloat)mat.restitution);
                content->DrawObject(objects[i].objectId, objects[i].lookId, objects[i].model);
            }
        }
        OpenGLState::UseProgram(0);

        //Compute sonar output
        glBindImageTexture(TEX_POSTPROCESS2, outputTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        sonarOutputShader->Use();
        sonarOutputShader->SetUniform("sonarInput", TEX_POSTPROCESS1);
        sonarOutputShader->SetUniform("sonarOutput", TEX_POSTPROCESS2);
        sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
        sonarOutputShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), 
                                                             randDist(randGen), 
                                                             randDist(randGen)));
        sonarOutputShader->SetUniform("noiseStddev", glm::vec2(0.1f, 0.05f));
        GLuint offset = 0;
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        for(size_t i=0; i<views.size(); ++i)
        {
            if((i == views.size()-1) && !allEqual) //Last one is different
                glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensity2Tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
            else
                glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_FALSE, i, GL_READ_ONLY, GL_RG32F);
            
            sonarOutputShader->SetUniform("beamOffset", offset);
            offset += views[i].nBeams;
            glDispatchCompute((GLuint)ceilf(views[i].nBeams/16.f), 1, 1);
        }
        OpenGLState::UseProgram(0);            
    }
    
    OpenGLState::BindFramebuffer(displayFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex);
    sonarVisualizeShader->Use();
    sonarVisualizeShader->SetUniform("texSonarData", TEX_POSTPROCESS1);
    sonarVisualizeShader->SetUniform("colormap", (GLint)cMap);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(fanVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (fanDiv+1)*2);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLFLS2::DrawLDR(GLuint destinationFBO)
{
    //Check if there is a need to display image on screen
    bool display = true;
    if(sonar != NULL)
        display = sonar->getDisplayOnScreen();
    
    //Draw on screen
    if(display)
    {
        if(0)
        {
            OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0,0,nBeams,nBeamSamples);

            if(views.size() == 1)
            {
                content->DrawTexturedQuad(0.f, 0.f, (GLfloat)nBeams, (GLfloat)nBeamSamples, inputRangeIntensityTex);
            }
            else
            {
                bool allEqual = views.front().nBeams == views.back().nBeams;
                GLuint offset = 0;
                for(size_t i=0; i<views.size()-(allEqual ? 0 : 1); ++i)
                {
                    content->DrawTexturedQuad((GLfloat)offset, 0.f, (GLfloat)views[i].nBeams, (GLfloat)nBeamSamples, 
                                                                                    inputRangeIntensityTex, i, true);
                    offset += views[i].nBeams;
                }
                
                if(!allEqual)
                {
                    content->DrawTexturedQuad((GLfloat)offset, 0.f, (GLfloat)views.back().nBeams, (GLfloat)nBeamSamples,
                                                                                    inputRangeIntensity2Tex);
                }
            }
            OpenGLState::BindFramebuffer(0);
        }
        else
        {
            OpenGLState::BindFramebuffer(destinationFBO);
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(displayTex);
            OpenGLState::BindFramebuffer(0);   
        }
    }
    
    //Copy texture to sonar buffer
    if(sonar != NULL && update)
    {
        /*OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, outputTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, sonar->getImageDataPointer(0));
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, displayTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, sonar->getDisplayDataPointer());
        OpenGLState::UnbindTexture(TEX_BASE);
        sonar->NewDataReady(0);*/
    }
    
    update = false;
}

///////////////////////// Static /////////////////////////////
void OpenGLFLS2::Init()
{
    sonarInputShader = new GLSLShader("sonarInput.frag", "sonarInput.vert");
    sonarInputShader->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader->AddUniform("M", ParameterType::MAT4);
    sonarInputShader->AddUniform("N", ParameterType::MAT3);
    sonarInputShader->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader->AddUniform("restitution", ParameterType::FLOAT);
    
    sonarVisualizeShader = new GLSLShader("sonarVisualize.frag", "printer.vert");
    sonarVisualizeShader->AddUniform("texSonarData", ParameterType::INT);
    sonarVisualizeShader->AddUniform("colormap", ParameterType::INT);
}

void OpenGLFLS2::Destroy()
{
    if(sonarInputShader != NULL) delete sonarInputShader;
    if(sonarVisualizeShader != NULL) delete sonarVisualizeShader;
}

}
