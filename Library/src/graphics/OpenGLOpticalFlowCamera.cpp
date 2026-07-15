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
//  OpenGLOpticalFlowCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/02/2024.
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOpticalFlowCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

std::unique_ptr<GLSLShader> OpenGLOpticalFlowCamera::opticalFlowCameraOutputShader;
std::unique_ptr<GLSLShader> OpenGLOpticalFlowCamera::opticalFlowVisualizeShader;
std::unique_ptr<GLSLShader> OpenGLOpticalFlowCamera::flipShader;

OpenGLOpticalFlowCamera::OpenGLOpticalFlowCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                          GLint originX, GLint originY, GLint width, GLint height,
                          GLfloat horizontalFOVDeg, glm::vec2 range, bool continuousUpdate)
 : OpenGLView(originX, originY, width, height), randDist_(0.f, 1.f)
{
    needsUpdate_ = false;
    continuous_ = continuousUpdate;
    newData_ = false;
    camera_ = nullptr;
    noiseVel_ = glm::vec2(0.f);
    maxVel_ = width/2.f;
    this->range_ = range;
    
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();
    
    fov_.x = horizontalFOVDeg/180.f*M_PI;
    fov_.y = 2.f * atanf( (GLfloat)viewportHeight_/(GLfloat)viewportWidth_ * tanf(fov_.x/2.f) );
    projection_ = glm::perspectiveFov(fov_.y, (GLfloat)viewportWidth_, (GLfloat)viewportHeight_, range.x, range.y);
    focalLength_ = ((GLfloat)viewportWidth_/2.f)/tanf(fov_.x/2.f);
    
    //Direct flow velocity output
    renderFlowTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    renderFlowTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                            GL_RG32F, GL_RG, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    renderDepthTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                           GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    std::vector<FBOTexture> fboTextures;
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderFlowTex_[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderFlowTex_[1]));
    fboTextures.push_back(FBOTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderDepthTex_));
    renderFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);

    //Flow visualization with color map
    displayFlowTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);

    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayFlowTex_));
    displayFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);

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
    
    glGenVertexArrays(1, &displayVAO_);
    OpenGLState::BindVertexArray(displayVAO_);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &displayVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);
}

OpenGLOpticalFlowCamera::~OpenGLOpticalFlowCamera()
{
    glDeleteTextures(1, &renderDepthTex_);
    glDeleteTextures(2, renderFlowTex_);
    glDeleteTextures(1, &displayFlowTex_);
    glDeleteFramebuffers(1, &renderFBO_);
    glDeleteFramebuffers(1, &displayFBO_);
    glDeleteVertexArrays(1, &displayVAO_);
    glDeleteBuffers(1, &displayVBO_);

    if(camera_ != nullptr)
    {
        glDeleteBuffers(1, &outputPBO_);
        glDeleteBuffers(1, &displayPBO_);
    }
}

void OpenGLOpticalFlowCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir_ = _dir;
    tempEye_ = _eye;
    tempUp_ = _up;
}

void OpenGLOpticalFlowCamera::UpdateTransform()
{
    eye_ = tempEye_;
    dir_ = tempDir_;
    up_ = tempUp_;
    SetupCamera();

    //Inform camera to run callback
    if(newData_)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera_->NewDataReady(src, 0);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
        GLfloat* src2 = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src2)
        {
            camera_->NewDataReady(src2, 1);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData_ = false;
    }
}

void OpenGLOpticalFlowCamera::SetupCamera()
{
    cameraTransform_ = glm::lookAt(eye_, eye_+dir_, up_);
}

glm::vec3 OpenGLOpticalFlowCamera::GetEyePosition() const
{
    return eye_;
}

glm::vec3 OpenGLOpticalFlowCamera::GetLookingDirection() const
{
    return dir_;
}

glm::vec3 OpenGLOpticalFlowCamera::GetUpDirection() const
{
    return up_;
}

glm::mat4 OpenGLOpticalFlowCamera::GetProjectionMatrix() const
{
    return projection_;
}

glm::mat4 OpenGLOpticalFlowCamera::GetViewMatrix() const
{
    return cameraTransform_;
}

GLfloat OpenGLOpticalFlowCamera::GetFOVX() const
{
    return fov_.x;
}

GLfloat OpenGLOpticalFlowCamera::GetFOVY() const
{
    return fov_.y;
}

GLfloat OpenGLOpticalFlowCamera::GetNearClip() const
{
    return range_.x;
}

GLfloat OpenGLOpticalFlowCamera::GetFarClip() const
{
    return range_.y;
}

void OpenGLOpticalFlowCamera::Update()
{
    needsUpdate_ = true;
}

bool OpenGLOpticalFlowCamera::needsUpdate()
{
    if(needsUpdate_)
    {
        needsUpdate_ = false;
        return enabled_;
    }
    else
        return false;
}

void OpenGLOpticalFlowCamera::setCamera(Camera* cam, unsigned int index)
{
    camera_ = cam;

    glGenBuffers(1, &outputPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * 2 * sizeof(GLfloat), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * 3 * sizeof(GLubyte), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLOpticalFlowCamera::setNoise(glm::vec2 velStdDev)
{
    noiseVel_ = velStdDev;
}

void OpenGLOpticalFlowCamera::setMaxVelocity(GLfloat v)
{
    maxVel_ = v;
}

ViewType OpenGLOpticalFlowCamera::getType() const
{
    return ViewType::OPTICAL_FLOW_CAMERA;
}

void OpenGLOpticalFlowCamera::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetCurrentView(this);
    content->SetDrawingMode(DrawingMode::RAW);
    
    //Optical flow velocity output
    OpenGLState::BindFramebuffer(renderFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 VP = GetProjectionMatrix() * GetViewMatrix();

    opticalFlowCameraOutputShader->Use();
    opticalFlowCameraOutputShader->SetUniform("FC", GetLogDepthConstant());
    opticalFlowCameraOutputShader->SetUniform("VR", glm::mat3(GetViewMatrix()));
    opticalFlowCameraOutputShader->SetUniform("d", GetLookingDirection());
    opticalFlowCameraOutputShader->SetUniform("c", glm::vec2(viewportWidth_/2.f, viewportHeight_/2.f));
    opticalFlowCameraOutputShader->SetUniform("f", focalLength_);
    opticalFlowCameraOutputShader->SetUniform("P_c", GetEyePosition());
    
    if(camera_ != nullptr)
    {
        Vector3 linear, angular;
        camera_->getSensorVelocity(linear, angular);
        opticalFlowCameraOutputShader->SetUniform("v_c", glVectorFromVector(linear));
        opticalFlowCameraOutputShader->SetUniform("w_c", glVectorFromVector(angular));
    }
    else
    {
        opticalFlowCameraOutputShader->SetUniform("v_c", glm::vec3(0.f));
        opticalFlowCameraOutputShader->SetUniform("w_c", glm::vec3(0.f));
    }

    for(size_t i=0; i<objects.size(); ++i)
    {
        if(objects[i].type != RenderableType::SOLID)
            continue;
        opticalFlowCameraOutputShader->SetUniform("MVP", VP * objects[i].model);
        opticalFlowCameraOutputShader->SetUniform("M", objects[i].model);
        opticalFlowCameraOutputShader->SetUniform("P_b", objects[i].cor);
        opticalFlowCameraOutputShader->SetUniform("v_b", objects[i].vel);
        opticalFlowCameraOutputShader->SetUniform("w_b", objects[i].avel);
        content->DrawObject(objects[i].objectId, -1, objects[i].model);
    }

    //Flip image
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderFlowTex_[0]);
    flipShader->Use();
    flipShader->SetUniform("texSource", TEX_POSTPROCESS1);
    static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    //Color mapped velocity display
    OpenGLState::BindFramebuffer(displayFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderFlowTex_[1]);
    opticalFlowVisualizeShader->Use();
    opticalFlowVisualizeShader->SetUniform("texFlow", TEX_POSTPROCESS1);
    opticalFlowVisualizeShader->SetUniform("maxVel", maxVel_);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLOpticalFlowCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
    //Check if there is a need to display image on screen
    bool display = true;
    unsigned int dispX, dispY;
    GLfloat dispScale;
    if(camera_ != nullptr)
        display = camera_->getDisplayOnScreen(dispX, dispY, dispScale);
    
    //Draw on screen
    if(display)
    {
        OpenGLContent* content = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent();
        int windowHeight = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getWindowHeight();
        int windowWidth = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getWindowWidth();
        OpenGLState::BindFramebuffer(destinationFBO);
        OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
        OpenGLState::DisableCullFace();
        content->DrawTexturedQuad(dispX, dispY+viewportHeight_*dispScale, viewportWidth_*dispScale, -viewportHeight_*dispScale, displayFlowTex_);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);
    }

    //Copy texture to camera buffer
    if(camera_ != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderFlowTex_[1]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, displayFlowTex_);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData_ = true;
    }
}

///////////////////////// Static /////////////////////////////
void OpenGLOpticalFlowCamera::Init()
{
    opticalFlowCameraOutputShader = std::make_unique<GLSLShader>("opticalFlow.frag", "opticalFlow.vert");
    opticalFlowCameraOutputShader->AddUniform("MVP", ParameterType::MAT4);
    opticalFlowCameraOutputShader->AddUniform("M", ParameterType::MAT4);
    opticalFlowCameraOutputShader->AddUniform("FC", ParameterType::FLOAT);
    opticalFlowCameraOutputShader->AddUniform("VR", ParameterType::MAT3);
    opticalFlowCameraOutputShader->AddUniform("d", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("P_b", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("v_b", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("w_b", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("P_c", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("v_c", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("w_c", ParameterType::VEC3);
    opticalFlowCameraOutputShader->AddUniform("c", ParameterType::VEC2);
    opticalFlowCameraOutputShader->AddUniform("f", ParameterType::FLOAT);

    opticalFlowVisualizeShader = std::make_unique<GLSLShader>("opticalFlowVisualize.frag");
    opticalFlowVisualizeShader->AddUniform("texFlow", ParameterType::INT);
    opticalFlowVisualizeShader->AddUniform("maxVel", ParameterType::FLOAT);

    flipShader = std::make_unique<GLSLShader>("verticalFlip.frag");
    flipShader->AddUniform("texSource", ParameterType::INT);
}

void OpenGLOpticalFlowCamera::Destroy()
{
    opticalFlowCameraOutputShader.reset();
    opticalFlowVisualizeShader.reset();
    flipShader.reset();
}

}
