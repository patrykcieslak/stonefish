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
//  OpenGLSonar.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/07/20.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSonar.h"

#include "core/Console.h"
#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

GLSLShader* OpenGLSonar::sonarInputShader_[2] = {nullptr, nullptr};
GLSLShader* OpenGLSonar::sonarVisualizeShader_[2] = {nullptr, nullptr};

OpenGLSonar::OpenGLSonar(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp, glm::uvec2 displayResolution, glm::vec2 range, SonarOutputFormat outputFormat)
    : OpenGLView(0, 0, displayResolution.x, displayResolution.y), randDist_(0.f, 1.f)
{
    needsUpdate_ = false;
    continuous = false;
    newData_ = false;
    range_ = range;
    gain_ = 1.f;
    settingsUpdated_ = true;
    outputPBO_ = 0;
    displayPBO_ = 0;
    fov_ = glm::vec2(1.0);
    cMap_ = ColorMap::GREEN_BLUE;
    outputFormat_ = outputFormat;
    SetupSonar(eyePosition, direction, sonarUp);
}

OpenGLSonar::~OpenGLSonar()
{
    glDeleteTextures(1, &inputRangeIntensityTex_);
    glDeleteRenderbuffers(1, &inputDepthRBO_);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(1, &displayTex_);
    glDeleteFramebuffers(1, &displayFBO_);
    glDeleteVertexArrays(1, &displayVAO_);
    glDeleteBuffers(1, &displayVBO_);
    if(outputPBO_ != 0) glDeleteBuffers(1, &outputPBO_);
    if(displayPBO_ != 0) glDeleteBuffers(1, &displayPBO_);
}

void OpenGLSonar::SetupSonar(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempEye_ = _eye;
    tempDir_ = _dir;
    tempUp_ = _up;
}

void OpenGLSonar::UpdateTransform()
{
    pendingCaptureTime_ = tempCaptureTime_;
    eye_ = tempEye_;
    dir_ = tempDir_;
    up_ = tempUp_;
    SetupSonar();
}

void OpenGLSonar::SetupSonar()
{
    sonarTransform_ = glm::lookAt(eye_, eye_+dir_, up_);
}

glm::vec3 OpenGLSonar::GetEyePosition() const
{
    return eye_;
}

glm::vec3 OpenGLSonar::GetLookingDirection() const
{
    return dir_;
}

glm::vec3 OpenGLSonar::GetUpDirection() const
{
    return up_;
}

glm::mat4 OpenGLSonar::GetProjectionMatrix() const
{
    return projection_;
}

glm::mat4 OpenGLSonar::GetViewMatrix() const
{
    return sonarTransform_;
}

GLfloat OpenGLSonar::GetNearClip() const
{
    return range_.x;
}

GLfloat OpenGLSonar::GetFarClip() const
{
    return range_.y;
}

GLfloat OpenGLSonar::GetFOVX() const
{
    return fov_.x;
}
        
GLfloat OpenGLSonar::GetFOVY() const
{
    return fov_.y;
}

void OpenGLSonar::Update()
{
    needsUpdate_ = true;
}

bool OpenGLSonar::needsUpdate()
{
    if(needsUpdate_)
    {
        needsUpdate_ = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLSonar::setColorMap(ColorMap cm)
{
    cMap_ = cm;
}

SonarOutputFormat OpenGLSonar::getOutputFormat() const
{
    return outputFormat_;
}

ViewType OpenGLSonar::getType() const
{
    return ViewType::SONAR;
}

///////////////////////// Static /////////////////////////////
void OpenGLSonar::Init()
{
    sonarInputShader_[0] = new GLSLShader("sonarInput.frag", "sonarInput.vert");
    sonarInputShader_[0]->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader_[0]->AddUniform("M", ParameterType::MAT4);
    sonarInputShader_[0]->AddUniform("N", ParameterType::MAT3);
    sonarInputShader_[0]->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader_[0]->AddUniform("restitution", ParameterType::FLOAT);
    
    sonarInputShader_[1] = new GLSLShader("sonarInputUv.frag", "sonarInputUv.vert");
    sonarInputShader_[1]->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader_[1]->AddUniform("M", ParameterType::MAT4);
    sonarInputShader_[1]->AddUniform("N", ParameterType::MAT3);
    sonarInputShader_[1]->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader_[1]->AddUniform("restitution", ParameterType::FLOAT);
    sonarInputShader_[1]->AddUniform("texNormal", ParameterType::INT);
    sonarInputShader_[1]->Use();
    sonarInputShader_[1]->SetUniform("texNormal", TEX_MAT_NORMAL);
    OpenGLState::UseProgram(0);
    
    sonarVisualizeShader_[0] = new GLSLShader("sonarVisualize.frag", "printer.vert");
    sonarVisualizeShader_[0]->AddUniform("texSonarData", ParameterType::INT);
    sonarVisualizeShader_[0]->AddUniform("colorMap", ParameterType::INT);

    sonarVisualizeShader_[1] = new GLSLShader("sonarVisualizeU32.frag", "printer.vert");
    sonarVisualizeShader_[1]->AddUniform("texSonarData", ParameterType::INT);
    sonarVisualizeShader_[1]->AddUniform("colorMap", ParameterType::INT);
}

void OpenGLSonar::Destroy()
{
    if(sonarInputShader_[0] != nullptr) delete sonarInputShader_[0];
    if(sonarInputShader_[1] != nullptr) delete sonarInputShader_[1];
    if(sonarVisualizeShader_[0] != nullptr) delete sonarVisualizeShader_[0];
    if(sonarVisualizeShader_[1] != nullptr) delete sonarVisualizeShader_[1];
}

}
