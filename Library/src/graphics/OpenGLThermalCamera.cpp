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
//  OpenGLThermalCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/02/2024.
//  Copyright (c) 2024-2025 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLThermalCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

GLSLShader* OpenGLThermalCamera::thermalVisualizeShader = nullptr;
GLSLShader* OpenGLThermalCamera::thermalOutputShader = nullptr;

OpenGLThermalCamera::OpenGLThermalCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                          GLint originX, GLint originY, GLint width, GLint height, GLfloat horizontalFOVDeg, 
                          glm::vec2 tempRange, glm::vec2 depthRange, bool continuousUpdate)
 : OpenGLView(originX, originY, width, height), camera(nullptr), _needsUpdate(false), newData(false), temperatureNoise(0.f), randDist(0.f, 1.f)
{
    continuous = continuousUpdate;
    this->depthRange = depthRange;
    temperatureRange = displayRange = tempRange;
    colorMap = ColorMap::JET;
    
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();
    
    fov.x = horizontalFOVDeg/180.f*M_PI;
    fov.y = 2.f * atanf( (GLfloat)viewportHeight/(GLfloat)viewportWidth * tanf(fov.x/2.f) );
    projection = glm::perspectiveFov(fov.y, (GLfloat)viewportWidth, (GLfloat)viewportHeight, depthRange.x, depthRange.y);
    
    //Direct temperature output
    renderTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                            GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    renderTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                            GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    renderDepthTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                           GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    std::vector<FBOTexture> fboTextures;
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex[0])); // Temperature
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderTex[1])); // Flipped temperature
    fboTextures.push_back(FBOTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderDepthTex));
    renderFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

    //Temperature visualization with color map
    displayTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);

    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayTex));
    displayFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

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
}

OpenGLThermalCamera::~OpenGLThermalCamera()
{
    glDeleteTextures(1, &renderDepthTex);
    glDeleteTextures(2, renderTex);
    glDeleteTextures(1, &displayTex);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteFramebuffers(1, &displayFBO);
    glDeleteVertexArrays(1, &displayVAO);
    glDeleteBuffers(1, &displayVBO);

    if(camera != nullptr)
    {
        glDeleteBuffers(1, &outputPBO);
        glDeleteBuffers(1, &displayPBO);
    }
}

void OpenGLThermalCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempEye = _eye;
    tempUp = _up;
}

void OpenGLThermalCamera::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupCamera();

    //Inform camera to run callback
    if(newData)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera->NewDataReady(src, 0);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
        GLfloat* src2 = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src2)
        {
            camera->NewDataReady(src2, 1);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData = false;
    }
}

void OpenGLThermalCamera::SetupCamera()
{
    cameraTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLThermalCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLThermalCamera::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLThermalCamera::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLThermalCamera::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLThermalCamera::GetViewMatrix() const
{
    return cameraTransform;
}

GLfloat OpenGLThermalCamera::GetNearClip() const
{
    return depthRange.x;
}

GLfloat OpenGLThermalCamera::GetFarClip() const
{
    return depthRange.y;
}

GLfloat OpenGLThermalCamera::GetFOVX() const
{
    return fov.x;
}
        
GLfloat OpenGLThermalCamera::GetFOVY() const
{
    return fov.y;
}

void OpenGLThermalCamera::Update()
{
    _needsUpdate = true;
}

bool OpenGLThermalCamera::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLThermalCamera::setCamera(Camera* cam, unsigned int index)
{
    camera = cam;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * sizeof(GLfloat), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3 * sizeof(GLubyte), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLThermalCamera::setNoise(GLfloat temperatureStdDev)
{
    temperatureNoise = temperatureStdDev;
}

void OpenGLThermalCamera::setDisplayRange(glm::vec2 tempRange)
{
    displayRange = tempRange;
}

void OpenGLThermalCamera::setColorMap(ColorMap cm)
{
    colorMap = cm;
}

ViewType OpenGLThermalCamera::getType() const
{
    return ViewType::THERMAL_CAMERA;
}

void OpenGLThermalCamera::ComputeOutput()
{
    //Flip image and saturate to measurement range
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderTex[0]);
    thermalOutputShader->Use();
    thermalOutputShader->SetUniform("texSource", TEX_POSTPROCESS1);
    thermalOutputShader->SetUniform("temperatureRange", temperatureRange);
    thermalOutputShader->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    thermalOutputShader->SetUniform("noiseStddev", temperatureNoise);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    //Color mapped temperature display
    OpenGLState::BindFramebuffer(displayFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderTex[1]);
    thermalVisualizeShader->Use();
    thermalVisualizeShader->SetUniform("texTemperature", TEX_POSTPROCESS1);
    thermalVisualizeShader->SetUniform("colorMap", (GLint)colorMap);
    thermalVisualizeShader->SetUniform("displayRange", displayRange);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLThermalCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
    //Check if there is a need to display image on screen
    bool display = true;
    unsigned int dispX, dispY;
    GLfloat dispScale;
    if(camera != nullptr)
        display = camera->getDisplayOnScreen(dispX, dispY, dispScale);
    
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
        content->DrawTexturedQuad(dispX, dispY+viewportHeight*dispScale, viewportWidth*dispScale, -viewportHeight*dispScale, displayTex);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);
    }

    //Copy texture to camera buffer
    if(camera != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderTex[1]);
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
}

///////////////////////// Static /////////////////////////////
void OpenGLThermalCamera::Init()
{   
    thermalOutputShader = new GLSLShader("thermalOutput.frag");
    thermalOutputShader->AddUniform("texSource", ParameterType::INT);
    thermalOutputShader->AddUniform("temperatureRange", ParameterType::VEC2);
    thermalOutputShader->AddUniform("noiseSeed", ParameterType::VEC3);
    thermalOutputShader->AddUniform("noiseStddev", ParameterType::FLOAT);
    
    thermalVisualizeShader = new GLSLShader("thermalVisualize.frag");
    thermalVisualizeShader->AddUniform("texTemperature", ParameterType::INT);
    thermalVisualizeShader->AddUniform("colorMap", ParameterType::INT);
    thermalVisualizeShader->AddUniform("displayRange", ParameterType::VEC2);
}

void OpenGLThermalCamera::Destroy()
{
    if(thermalOutputShader != nullptr) delete thermalOutputShader;
    if(thermalVisualizeShader != nullptr) delete thermalVisualizeShader;
}

}
