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

GLSLShader* OpenGLSonar::sonarInputShader[2] = {nullptr, nullptr};
GLSLShader* OpenGLSonar::sonarVisualizeShader = nullptr;

OpenGLSonar::OpenGLSonar(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 sonarUp, glm::uvec2 displayResolution, glm::vec2 range_)
    : OpenGLView(0, 0, displayResolution.x, displayResolution.y), randDist(0.f, 1.f)
{
    _needsUpdate = false;
    continuous = false;
    newData = false;
    range = range_;
    gain = 1.f;
    settingsUpdated = true;
    outputPBO = 0;
    displayPBO = 0;
    cMap = ColorMap::GREEN_BLUE;
    SetupSonar(eyePosition, direction, sonarUp);
}

OpenGLSonar::~OpenGLSonar()
{
    glDeleteTextures(1, &inputRangeIntensityTex);
    glDeleteRenderbuffers(1, &inputDepthRBO);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(1, &displayTex);
    glDeleteFramebuffers(1, &displayFBO);
    glDeleteVertexArrays(1, &displayVAO);
    glDeleteBuffers(1, &displayVBO);
    if(outputPBO != 0) glDeleteBuffers(1, &outputPBO);
    if(displayPBO != 0) glDeleteBuffers(1, &displayPBO);
}

void OpenGLSonar::SetupSonar(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempEye = _eye;
    tempDir = _dir;
    tempUp = _up;
}

void OpenGLSonar::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupSonar();
}

void OpenGLSonar::SetupSonar()
{
    sonarTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLSonar::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLSonar::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLSonar::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLSonar::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLSonar::GetViewMatrix() const
{
    return sonarTransform;
}

GLfloat OpenGLSonar::GetFarClip() const
{
    return range.y;
}

void OpenGLSonar::Update()
{
    _needsUpdate = true;
}

bool OpenGLSonar::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLSonar::setColorMap(ColorMap cm)
{
    cMap = cm;
}

ViewType OpenGLSonar::getType()
{
    return ViewType::SONAR;
}

///////////////////////// Static /////////////////////////////
void OpenGLSonar::Init()
{
    sonarInputShader[0] = new GLSLShader("sonarInput.frag", "sonarInput.vert");
    sonarInputShader[0]->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader[0]->AddUniform("M", ParameterType::MAT4);
    sonarInputShader[0]->AddUniform("N", ParameterType::MAT3);
    sonarInputShader[0]->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader[0]->AddUniform("restitution", ParameterType::FLOAT);
    
    sonarInputShader[1] = new GLSLShader("sonarInputUv.frag", "sonarInputUv.vert");
    sonarInputShader[1]->AddUniform("MVP", ParameterType::MAT4);
    sonarInputShader[1]->AddUniform("M", ParameterType::MAT4);
    sonarInputShader[1]->AddUniform("N", ParameterType::MAT3);
    sonarInputShader[1]->AddUniform("eyePos", ParameterType::VEC3);
    sonarInputShader[1]->AddUniform("restitution", ParameterType::FLOAT);
    sonarInputShader[1]->AddUniform("texNormal", ParameterType::INT);
    sonarInputShader[1]->Use();
    sonarInputShader[1]->SetUniform("texNormal", TEX_MAT_NORMAL);
    OpenGLState::UseProgram(0);
    
    sonarVisualizeShader = new GLSLShader("sonarVisualize.frag", "printer.vert");
    sonarVisualizeShader->AddUniform("texSonarData", ParameterType::INT);
    sonarVisualizeShader->AddUniform("colormap", ParameterType::INT);
}

void OpenGLSonar::Destroy()
{
    if(sonarInputShader[0] != nullptr) delete sonarInputShader[0];
    if(sonarInputShader[1] != nullptr) delete sonarInputShader[1];
    if(sonarVisualizeShader != nullptr) delete sonarVisualizeShader;
}

}
