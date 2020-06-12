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

GLSLShader* OpenGLFLS::sonarInputShader = NULL;
GLSLShader* OpenGLFLS::sonarVisualizeShader = NULL;

OpenGLFLS::OpenGLFLS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
                       GLint originX, GLint originY, GLfloat horizontalFOVDeg, GLfloat verticalFOVDeg, 
                       GLuint numOfBeams, GLuint numOfBins, glm::vec2 range_, bool continuousUpdate)
: OpenGLView(originX, originY, 2*numOfBins, numOfBins), randDist(0.f, 1.f)
{
    _needsUpdate = false;
    update = false;
    continuous = continuousUpdate;
    newData = false;
    sonar = NULL;
    range = range_;
    nBeams = numOfBeams;
    nBins = numOfBins;
    nBeamSamples = (GLuint)ceilf(verticalFOVDeg * range.y * FLS_VRES_FACTOR);
    cMap = ColorMap::COLORMAP_HOT;
    
    SetupSonar(eyePosition, direction, sonarUp);
    UpdateTransform();
    
    fov.x = glm::radians(horizontalFOVDeg);
    fov.y = glm::radians(verticalFOVDeg);
    GLfloat hFactor = sinf(fov.x/2.f);
    viewportWidth = (GLint)ceilf(2.f*hFactor*numOfBins);

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
    for(GLuint i=0; i<nViews; ++i)
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, inputRangeIntensityTex, 0, i);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");

    //Setup matrices
    GLfloat viewFovCorr = (GLfloat)nViewBeams/(GLfloat)nBeams * fov.x;
    glm::mat4 proj;
    proj[0] = glm::vec4(range.x/(range.x*tanf(viewFovCorr/2.f)), 0.f, 0.f, 0.f);
    proj[1] = glm::vec4(0.f, range.x/(range.x*tanf(fov.y/2.f)), 0.f, 0.f);
    proj[2] = glm::vec4(0.f, 0.f, -(range.y + range.x)/(range.y-range.x), -1.f);
    proj[3] = glm::vec4(0.f, 0.f, -2.f*range.y*range.x/(range.y-range.x), 0.f);
    
    GLfloat viewFovAcc = 0.f;
    for(size_t i=0; i<views.size(); ++i)
    {
        views[i].projection = proj;
        views[i].view = glm::rotate(-fov.x/2.f + viewFovAcc + viewFovCorr/2.f, glm::vec3(0.f,1.f,0.f));
        
        GLfloat viewFov = (GLfloat)views[i].nBeams/(GLfloat)nBeams * fov.x;
        viewFovAcc += viewFov;
    }
    
    //Output shader: sonar range data
    outputTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(nBeams, nBins, 1), 
                                               GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    glGenFramebuffers(1, &outputFBO);
    OpenGLState::BindFramebuffer(outputFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTex, 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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
    
    //Flipped vertically to account for OpenGL window coordinates
    for(GLuint i=0; i<fanDiv+1; ++i)
    {
        GLfloat alpha = fov.x/2.f - i/(GLfloat)fanDiv * fov.x;
        //Min range edge
        fanData[i*2][0] = -Rmin*sinf(alpha)*1.f/hFactor;
        fanData[i*2][1] = (1.f-Rmin*cosf(alpha))*2.f-1.f;
        fanData[i*2][2] = i/(GLfloat)fanDiv;
        fanData[i*2][3] = 0.f;
        //Max range edge
        fanData[i*2+1][0] = -sinf(alpha)*1.f/hFactor;
        fanData[i*2+1][1] = (1.f-cosf(alpha))*2.f-1.f;
        fanData[i*2+1][2] = i/(GLfloat)fanDiv;
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
    sonarOutputShader->AddUniform("beams", ParameterType::UVEC2);
    sonarOutputShader->AddUniform("range", ParameterType::VEC3);
    sonarOutputShader->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarOutputShader->AddUniform("noiseStddev", ParameterType::VEC2);

    sonarOutputShader->Use();
    sonarOutputShader->SetUniform("sonarInput", TEX_POSTPROCESS1);
    sonarOutputShader->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    sonarOutputShader->SetUniform("beams", glm::uvec2(views.front().nBeams, views.back().nBeams));
    sonarOutputShader->SetUniform("range", glm::vec3(range.x, range.y, (range.y-range.x)/(GLfloat)nBins));
    OpenGLState::UseProgram(0);
}

OpenGLFLS::~OpenGLFLS()
{
    delete sonarOutputShader;
    glDeleteTextures(1, &inputRangeIntensityTex);
    glDeleteRenderbuffers(1, &inputDepthRBO);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(1, &outputTex);
    glDeleteFramebuffers(1, &outputFBO);
    glDeleteTextures(1, &displayTex);
    glDeleteFramebuffers(1, &displayFBO);
    glDeleteBuffers(1, &fanBuf);
    glDeleteVertexArrays(1, &fanVAO);

    if(sonar != NULL)
    {
        glDeleteBuffers(1, &outputPBO);
        glDeleteBuffers(1, &displayPBO);
    }
}

void OpenGLFLS::SetupSonar(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempEye = _eye;
    tempUp = _up;
}

void OpenGLFLS::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupSonar();

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
        GLfloat* src2 = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src2)
        {
            sonar->NewDataReady(src2, 1);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData = false;
    }
}

void OpenGLFLS::SetupSonar()
{
    sonarTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLFLS::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLFLS::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLFLS::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLFLS::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLFLS::GetViewMatrix() const
{
    return sonarTransform;
}

GLfloat OpenGLFLS::GetFarClip() const
{
    return range.y;
}

void OpenGLFLS::Update()
{
    _needsUpdate = true;
    update = true;
}

bool OpenGLFLS::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLFLS::setColorMap(ColorMap cm)
{
    cMap = cm;
}

void OpenGLFLS::setSonar(FLS* s)
{
    sonar = s;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, nBeams * nBins * sizeof(GLfloat), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

ViewType OpenGLFLS::getType()
{
    return SONAR;
}

void OpenGLFLS::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, nViewBeams, nBeamSamples);
    sonarInputShader->Use();
    sonarInputShader->SetUniform("eyePos", GetEyePosition());
    for(size_t i=0; i<views.size(); ++i) //For each of the sonar views
    {
        //Clear color and depth for particular framebuffer layer
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + (GLuint)i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Calculate view transform
        glm::mat4 VP = views[i].projection * views[i].view * GetViewMatrix();
        //Draw objects
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
    OpenGLState::UseProgram(0);

    //Compute sonar output
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    sonarOutputShader->Use();
    sonarOutputShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    sonarOutputShader->SetUniform("noiseStddev", glm::vec2(0.05f, 0.02f));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nViewBeams/16.f), (GLuint)views.size(), 1);
    OpenGLState::UseProgram(0);            
    
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

void OpenGLFLS::DrawLDR(GLuint destinationFBO)
{
    //Check if there is a need to display image on screen
    bool display = true;
    if(sonar != NULL)
        display = sonar->getDisplayOnScreen();
    
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
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0,0,viewportWidth,viewportHeight);
            content->DrawTexturedSAQ(displayTex);
            OpenGLState::BindFramebuffer(0);   
        }
    }
    
    //Copy texture to sonar buffer
    if(sonar != NULL && update)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, displayTex);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData = true;
    }
    
    update = false;
}

///////////////////////// Static /////////////////////////////
void OpenGLFLS::Init()
{
    sonarInputShader = new GLSLShader("sonarInput.frag", "sonarInput.vert");
    sonarInputShader->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader->AddUniform("M", ParameterType::MAT4);
    sonarInputShader->AddUniform("N", ParameterType::MAT3);
    sonarInputShader->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader->AddUniform("restitution", ParameterType::FLOAT);
    
    sonarVisualizeShader = new GLSLShader("sonarVisualize.frag", "printer.vert");
    sonarVisualizeShader->AddUniform("texSonarData", ParameterType::INT);
    sonarVisualizeShader->AddUniform("colo-(allEqual ? 0 : 1)rmap", ParameterType::INT);
}

void OpenGLFLS::Destroy()
{
    if(sonarInputShader != NULL) delete sonarInputShader;
    if(sonarVisualizeShader != NULL) delete sonarVisualizeShader;
}

}
