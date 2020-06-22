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
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSSS.h"

#include <algorithm>
#include "core/Console.h"
#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/SSS.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

#define SSS_VRES_FACTOR 1.f
#define SSS_HRES_FACTOR 100.f

namespace sf
{

GLSLShader* OpenGLSSS::sonarInputShader = NULL;
GLSLShader* OpenGLSSS::sonarShiftShader = NULL;
GLSLShader* OpenGLSSS::sonarVisualizeShader = NULL;

OpenGLSSS::OpenGLSSS(glm::vec3 centerPosition, glm::vec3 direction, glm::vec3 forward,
                     GLint originX, GLint originY, GLfloat verticalBeamWidthDeg, GLfloat horizontalBeamWidthDeg, 
                     GLuint numOfBins, GLuint numOfLines, GLfloat verticalTiltDeg, glm::vec2 range_, bool continuousUpdate)
: OpenGLView(originX, originY, numOfBins, numOfLines), randDist(0.f, 1.f)
{
    _needsUpdate = false;
    update = false;
    continuous = continuousUpdate;
    newData = false;
    sonar = NULL;
    range = range_;
    tilt = glm::radians(verticalTiltDeg);
    fov.x = glm::radians(verticalBeamWidthDeg);
    fov.y = glm::radians(horizontalBeamWidthDeg);
    GLfloat binRange = 2.f*(range.y-range.x)/(GLfloat)viewportWidth;
    nBeamSamples.x = glm::max((GLuint)ceilf(verticalBeamWidthDeg * 1.f/binRange * SSS_VRES_FACTOR), (GLuint)2048);
    nBeamSamples.y = glm::max((GLuint)ceilf(horizontalBeamWidthDeg * SSS_HRES_FACTOR), (GLuint)2048);
    cMap = ColorMap::COLORMAP_HOT;
    
    SetupSonar(centerPosition, direction, forward);
    UpdateTransform();
    
    //Setup matrices
    projection[0] = glm::vec4(range.x/(range.x*tanf(fov.x/2.f)), 0.f, 0.f, 0.f);
    projection[1] = glm::vec4(0.f, range.x/(range.x*tanf(fov.y/2.f)), 0.f, 0.f);
    projection[2] = glm::vec4(0.f, 0.f, -(range.y + range.x)/(range.y-range.x), -1.f);
    projection[3] = glm::vec4(0.f, 0.f, -2.f*range.y*range.x/(range.y-range.x), 0.f);
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
                                                  GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    outputTex[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 1), 
                                                  GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    pingpong = 0;
    //Visualisation shader: sonar data color mapped
    glGenTextures(1, &displayTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, displayTex);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); //RGB image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);
    std::vector<FBOTexture> textures;
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayTex));
    displayFBO = OpenGLContent::GenerateFramebuffer(textures);
    
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
    sonarOutputShader[1]->Use();
    sonarOutputShader[1]->SetUniform("sonarHist", TEX_POSTPROCESS1);
    sonarOutputShader[1]->SetUniform("sonarOutput", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);  
}

OpenGLSSS::~OpenGLSSS()
{
    delete sonarOutputShader[0];
    delete sonarOutputShader[1];
    glDeleteTextures(1, &inputRangeIntensityTex);
    glDeleteRenderbuffers(1, &inputDepthRBO);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(3, outputTex);
    glDeleteTextures(1, &displayTex);
    glDeleteFramebuffers(1, &displayFBO);

    if(sonar != NULL)
    {
        glDeleteBuffers(1, &outputPBO);
        glDeleteBuffers(1, &displayPBO);
    }
}

void OpenGLSSS::SetupSonar(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempCenter = _eye;
    tempForward = _up;
}

void OpenGLSSS::UpdateTransform()
{
    center = tempCenter;
    dir = tempDir;
    forward = tempForward;
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

void OpenGLSSS::SetupSonar()
{
    sonarTransform = glm::lookAt(center, center+dir, forward);
}

glm::vec3 OpenGLSSS::GetEyePosition() const
{
    return center;
}

glm::vec3 OpenGLSSS::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLSSS::GetUpDirection() const
{
    return forward;
}

glm::mat4 OpenGLSSS::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLSSS::GetViewMatrix() const
{
    return sonarTransform;
}

GLfloat OpenGLSSS::GetFarClip() const
{
    return range.y;
}

void OpenGLSSS::Update()
{
    _needsUpdate = true;
    update = true;
}

bool OpenGLSSS::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLSSS::setColorMap(ColorMap cm)
{
    cMap = cm;
}

void OpenGLSSS::setSonar(SSS* s)
{
    sonar = s;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * sizeof(GLfloat), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

ViewType OpenGLSSS::getType()
{
    return ViewType::SSS;
}

void OpenGLSSS::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    //Generate sonar input
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, nBeamSamples.x, nBeamSamples.y);
    sonarInputShader->Use();
    sonarInputShader->SetUniform("eyePos", center);
    for(size_t i=0; i<2; ++i) //For each of the sonar views
    {
        //Compute matrices
        glm::mat4 V = views[i] * GetViewMatrix();
        glm::mat4 VP = projection * V;
        //Clear color and depth for particular framebuffer layer
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + (GLuint)i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    
    //Compute sonar output histogram
    glBindImageTexture(TEX_POSTPROCESS1, inputRangeIntensityTex, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
    sonarOutputShader[0]->Use();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(nBeamSamples.x/64.f), 2, 1);
    
    //Shift old sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex[pingpong + 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(TEX_POSTPROCESS2, outputTex[1-pingpong + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    sonarShiftShader->Use();
    glDispatchCompute((GLuint)ceilf(viewportWidth/16.f), (GLuint)ceilf(viewportHeight/16.f), 1);
    
    //Postprocess sonar output
    glBindImageTexture(TEX_POSTPROCESS1, outputTex[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
    sonarOutputShader[1]->Use();
    sonarOutputShader[1]->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    sonarOutputShader[1]->SetUniform("noiseStddev", glm::vec2(0.02f, 0.02f));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute((GLuint)ceilf(viewportWidth/2.f/64.f), 2, 1);
    
    OpenGLState::BindFramebuffer(displayFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, outputTex[1-pingpong + 1]);
    sonarVisualizeShader->Use();
    sonarVisualizeShader->SetUniform("texSonarData", TEX_POSTPROCESS2);
    sonarVisualizeShader->SetUniform("colormap", (GLint)cMap);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    content->DrawSAQ();
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);

    ++pingpong;
    if(pingpong > 1)
        pingpong = 0;
}

void OpenGLSSS::DrawLDR(GLuint destinationFBO)
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
            content->SetViewportSize(nBeamSamples.x/2, nBeamSamples.y/4);
            OpenGLState::BindFramebuffer(destinationFBO);
            OpenGLState::Viewport(0, 0, nBeamSamples.x/2, nBeamSamples.y/4);
            content->DrawTexturedQuad(0.f, 0.f, (GLfloat)nBeamSamples.x/4, (GLfloat)nBeamSamples.y/4, inputRangeIntensityTex, 0, true);
            content->DrawTexturedQuad((GLfloat)nBeamSamples.x/4, 0.f, (GLfloat)nBeamSamples.x/4, (GLfloat)nBeamSamples.y/4, inputRangeIntensityTex, 1, true);
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
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, outputTex[pingpong+1]);
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
void OpenGLSSS::Init()
{
    sonarInputShader = new GLSLShader("sonarInput.frag", "sonarInput.vert");
    sonarInputShader->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader->AddUniform("M", ParameterType::MAT4);
    sonarInputShader->AddUniform("N", ParameterType::MAT3);
    sonarInputShader->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader->AddUniform("restitution", ParameterType::FLOAT);
    
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "sonarShift.comp"));
    sonarShiftShader = new GLSLShader(sources);
    sonarShiftShader->AddUniform("sonarOutputIn", ParameterType::INT);
    sonarShiftShader->AddUniform("sonarOutputOut", ParameterType::INT);
    sonarShiftShader->Use();
    sonarShiftShader->SetUniform("sonarOutputIn", TEX_POSTPROCESS1);
    sonarShiftShader->SetUniform("sonarOutputOut", TEX_POSTPROCESS2);
    OpenGLState::UseProgram(0);

    sonarVisualizeShader = new GLSLShader("sonarVisualize.frag");
    sonarVisualizeShader->AddUniform("texSonarData", ParameterType::INT);
    sonarVisualizeShader->AddUniform("colormap", ParameterType::INT);
}

void OpenGLSSS::Destroy()
{
    if(sonarInputShader != NULL) delete sonarInputShader;
    if(sonarShiftShader != NULL) delete sonarShiftShader;
    if(sonarVisualizeShader != NULL) delete sonarVisualizeShader;
}

}
