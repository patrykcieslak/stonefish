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
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

GLSLShader* OpenGLFLS::sonarInputShader = NULL;
GLSLShader* OpenGLFLS::sonarOutputShader = NULL;
GLSLShader* OpenGLFLS::sonarVisualizeShader = NULL;

OpenGLFLS::OpenGLFLS(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp,
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
    inputWidth = numOfBeams * beamHPix;
    inputHeight = beamVPix;
    
    SetupSonar(eyePosition, direction, sonarUp);
    UpdateTransform();
    
    fov.x = horizontalFOVDeg/180.f*M_PI;
    fov.y = verticalFOVDeg/180.f*M_PI;
    projection[0] = glm::vec4(range.x/(range.x*tanf(fov.x/2.f)), 0.f, 0.f, 0.f);
    projection[1] = glm::vec4(0.f, range.x/(range.x*tanf(fov.y/2.f)), 0.f, 0.f);
    projection[2] = glm::vec4(0.f, 0.f, -(range.y + range.x)/(range.y-range.x), -1.f);
    projection[3] = glm::vec4(0.f, 0.f, -2.f*range.y*range.x/(range.y-range.x), 0.f);
    GLfloat hFactor = sinf(fov.x/2.f);
    viewportWidth = (GLint)ceilf(2.f*hFactor*numOfBins);
    
    //Input shader: range + echo intensity
    //float maxAnisotropy;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    
    glGenTextures(1, &inputRangeIntensityTex);
    glBindTexture(GL_TEXTURE_2D, inputRangeIntensityTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, inputWidth, inputHeight, 0, GL_RG, GL_FLOAT, NULL); //2 float channels
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //No interpolation!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //No interpolation!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenRenderbuffers(1, &inputDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, inputDepthRBO); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, inputWidth, inputHeight);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, inputDepthRBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, inputRangeIntensityTex, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar input FBO initialization failed!");
        
    //Output shader: sonar range data
    glGenTextures(1, &outputTex);
    glBindTexture(GL_TEXTURE_2D, outputTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, nBeams, nBins, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Interpolation for fan rendering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //Interpolation for fan rendering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &outputFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTex, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar output FBO initialization failed!");
        
    //Sonar display fan
    glGenTextures(1, &displayTex);
    glBindTexture(GL_TEXTURE_2D, displayTex);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); //RGB image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //No interpolation!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //No interpolation!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &displayFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, displayFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, displayTex, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sonar display FBO initialization failed!");
    
    glGenVertexArrays(1, &fanVAO);
    glBindVertexArray(fanVAO);
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
    
    glBindVertexArray(0);
}

OpenGLFLS::~OpenGLFLS()
{
    glDeleteTextures(1, &inputRangeIntensityTex);
    glDeleteRenderbuffers(1, &inputDepthRBO);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(1, &outputTex);
    glDeleteFramebuffers(1, &outputFBO);
    glDeleteTextures(1, &displayTex);
    glDeleteFramebuffers(1, &displayFBO);
    glDeleteBuffers(1, &fanBuf);
    glDeleteVertexArrays(1, &fanVAO);
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

void OpenGLFLS::Update()
{
    _needsUpdate = true;
}

bool OpenGLFLS::needsUpdate()
{
    update = _needsUpdate;
    _needsUpdate = false;
    return update && enabled;
}

void OpenGLFLS::setSonar(FLS* s)
{
    sonar = s;
}

ViewType OpenGLFLS::getType()
{
    return SONAR;
}

void OpenGLFLS::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetDrawingMode(DrawingMode::RAW);
    
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glViewport(0, 0, inputWidth, inputHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    sonarInputShader->Use();
    sonarInputShader->SetUniform("eyePos", GetEyePosition());
    glm::mat4 VP = GetProjectionMatrix() * GetViewMatrix();
    
    for(size_t i=0; i<objects.size(); ++i)
    {
        if(objects[i].type == RenderableType::SOLID)
        {
            glm::mat4 M = objects[i].model;
            Material mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(objects[i].materialName);
            sonarInputShader->SetUniform("MVP", VP * M);
            sonarInputShader->SetUniform("M", M);
            sonarInputShader->SetUniform("N", glm::mat3(glm::transpose(glm::inverse(M))));
            sonarInputShader->SetUniform("restitution", (GLfloat)mat.restitution);
            content->DrawObject(objects[i].objectId, objects[i].lookId, objects[i].model);
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, nBeams, nBins);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, inputRangeIntensityTex);
    
    GLfloat d = 0.5f/tanf(fov.x/2.f);
    GLint beamSamples = inputHeight; //No linear interpolation means no sense to oversample
    sonarOutputShader->Use();
    sonarOutputShader->SetUniform("texRangeIntensity", TEX_POSTPROCESS1);
    sonarOutputShader->SetUniform("beamSamples", (GLint)beamSamples);
    sonarOutputShader->SetUniform("beamSampleStep", 1.f/(GLfloat)(beamSamples-1));
    sonarOutputShader->SetUniform("rangeMin", range.x);
    sonarOutputShader->SetUniform("binRangeStep", (range.y-range.x)/(GLfloat)(nBins));
    sonarOutputShader->SetUniform("halfFOV", fov.x/2.f);
    sonarOutputShader->SetUniform("beamAngleStep", fov.x/(GLfloat)nBeams);
    sonarOutputShader->SetUniform("d", d);
    sonarOutputShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    sonarOutputShader->SetUniform("noiseStddev", glm::vec2(0.1f, 0.05f));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    glBindFramebuffer(GL_FRAMEBUFFER, displayFBO);
    glViewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, outputTex);
     
    sonarVisualizeShader->Use();
    sonarVisualizeShader->SetUniform("texSonarData", TEX_POSTPROCESS1);
    glBindVertexArray(fanVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (fanDiv+1)*2);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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
        glBindFramebuffer(GL_FRAMEBUFFER, destinationFBO);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(displayTex);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    //Copy texture to sonar buffer
    if(sonar != NULL && update)
    {
        glBindTexture(GL_TEXTURE_2D, outputTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, sonar->getImageDataPointer(0));
        glBindTexture(GL_TEXTURE_2D, displayTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, sonar->getDisplayDataPointer());
        glBindTexture(GL_TEXTURE_2D, 0);
        sonar->NewDataReady(0);
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
    
    sonarOutputShader = new GLSLShader("sonarOutput.frag");
    sonarOutputShader->AddUniform("texRangeIntensity", ParameterType::INT);
    sonarOutputShader->AddUniform("beamSamples", ParameterType::INT);
    sonarOutputShader->AddUniform("beamSampleStep", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("rangeMin", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("binRangeStep", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("halfFOV", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("beamAngleStep", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("d", ParameterType::FLOAT);
    sonarOutputShader->AddUniform("noiseSeed", ParameterType::VEC3);
    sonarOutputShader->AddUniform("noiseStddev", ParameterType::VEC2);
    
    sonarVisualizeShader = new GLSLShader("sonarVisualize.frag", "printer.vert");
    sonarVisualizeShader->AddUniform("texSonarData", ParameterType::INT);
}

void OpenGLFLS::Destroy()
{
    if(sonarInputShader != NULL) delete sonarInputShader;
    if(sonarOutputShader != NULL) delete sonarOutputShader;
    if(sonarVisualizeShader != NULL) delete sonarVisualizeShader;
}

}
