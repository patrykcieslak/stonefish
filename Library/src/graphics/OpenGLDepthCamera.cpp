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
//  OpenGLDepthCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018-2024 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDepthCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

GLSLShader** OpenGLDepthCamera::depthCameraOutputShader = nullptr;
GLSLShader* OpenGLDepthCamera::depthVisualizeShader = nullptr;

OpenGLDepthCamera::OpenGLDepthCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                                     GLint originX, GLint originY, GLint width, GLint height,
                                     GLfloat horizontalFOVDeg, GLfloat minDepth, GLfloat maxDepth,
                                     bool continuousUpdate, bool useRanges, GLfloat verticalFOVDeg)
 : OpenGLView(originX, originY, width, height), randDist_(0.f, 1.f)
{
    needsUpdate_ = false;
    continuous_ = continuousUpdate;
    newData_ = false;
    camera_ = nullptr;
    noiseDepth_ = 0.f;
    idx_ = 0;
    range_.x = minDepth;
    range_.y = maxDepth;
    usesRanges_ = useRanges;
    linearDepthPBO_ = 0;
    
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();
    
    fov_.x = horizontalFOVDeg/180.f*M_PI;
    
    if(verticalFOVDeg > 0.f)
    {
        fov_.y = verticalFOVDeg/180.f*M_PI;
        projection_[0] = glm::vec4(range_.x/(range_.x*tanf(fov_.x/2.f)), 0.f, 0.f, 0.f);
        projection_[1] = glm::vec4(0.f, range_.x/(range_.x*tanf(fov_.y/2.f)), 0.f, 0.f);
        projection_[2] = glm::vec4(0.f, 0.f, -(range_.y + range_.x)/(range_.y-range_.x), -1.f);
        projection_[3] = glm::vec4(0.f, 0.f, -2.f*range_.y*range_.x/(range_.y-range_.x), 0.f);
    }
    else
    {
        fov_.y = 2.f * atanf( (GLfloat)viewportHeight_/(GLfloat)viewportWidth_ * tanf(fov_.x/2.f) );
        projection_ = glm::perspectiveFov(fov_.y, (GLfloat)viewportWidth_, (GLfloat)viewportHeight_, range_.x, range_.y);
    }
    
    //Render depth
    glGenTextures(1, &renderDepthTex_);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, renderDepthTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, viewportWidth_, viewportHeight_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);

    glGenFramebuffers(1, &renderFBO_);
    OpenGLState::BindFramebuffer(renderFBO_);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderDepthTex_, 0);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Depth Render FBO initialization failed!");
    
    //Linear depth
    glGenTextures(1, &linearDepthTex_);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, linearDepthTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth_, viewportHeight_, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);

    glGenFramebuffers(1, &linearDepthFBO_);
    OpenGLState::BindFramebuffer(linearDepthFBO_);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, linearDepthTex_, 0);
        
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Linear Depth FBO initialization failed!");
    
    OpenGLState::BindFramebuffer(0);
}

OpenGLDepthCamera::~OpenGLDepthCamera()
{
    glDeleteTextures(1, &renderDepthTex_);
    glDeleteFramebuffers(1, &renderFBO_);
    glDeleteTextures(1, &linearDepthTex_);
    glDeleteFramebuffers(1, &linearDepthFBO_);

    if(camera_ != nullptr)
    {
        glDeleteBuffers(1, &linearDepthPBO_);
    }
}

void OpenGLDepthCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir_ = _dir;
    tempEye_ = _eye;
    tempUp_ = _up;
}

void OpenGLDepthCamera::UpdateTransform()
{
    eye_ = tempEye_;
    dir_ = tempDir_;
    up_ = tempUp_;
    SetupCamera();

    //Inform camera to run callback
    if(newData_)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, linearDepthPBO_);
        GLfloat* src = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera_->NewDataReady(src, idx_);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData_ = false;
    }
}

void OpenGLDepthCamera::SetupCamera()
{
    cameraTransform_ = glm::lookAt(eye_, eye_+dir_, up_);
}

glm::vec3 OpenGLDepthCamera::GetEyePosition() const
{
    return eye_;
}

glm::vec3 OpenGLDepthCamera::GetLookingDirection() const
{
    return dir_;
}

glm::vec3 OpenGLDepthCamera::GetUpDirection() const
{
    return up_;
}

glm::mat4 OpenGLDepthCamera::GetProjectionMatrix() const
{
    return projection_;
}

glm::mat4 OpenGLDepthCamera::GetViewMatrix() const
{
    return cameraTransform_;
}

GLfloat OpenGLDepthCamera::GetNearClip() const
{
    return range_.x;
}

GLfloat OpenGLDepthCamera::GetFarClip() const
{
    return range_.y;
}

GLfloat OpenGLDepthCamera::GetFOVX() const
{
    return fov_.x;
}
        
GLfloat OpenGLDepthCamera::GetFOVY() const
{
    return fov_.y;
}

void OpenGLDepthCamera::Update()
{
    needsUpdate_ = true;
}

bool OpenGLDepthCamera::needsUpdate()
{
    if(needsUpdate_)
    {
        needsUpdate_ = false;
        return enabled_;
    }
    else
        return false;
}

void OpenGLDepthCamera::setCamera(Camera* cam, unsigned int index)
{
    camera_ = cam;
    idx_ = index;

    glGenBuffers(1, &linearDepthPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, linearDepthPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * sizeof(GLfloat), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLDepthCamera::setNoise(GLfloat depthStdDev)
{
    noiseDepth_ = depthStdDev;
}

ViewType OpenGLDepthCamera::getType() const
{
    return ViewType::DEPTH_CAMERA;
}

void OpenGLDepthCamera::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetCurrentView(this);
    content->SetDrawingMode(DrawingMode::SHADOW);
    OpenGLState::BindFramebuffer(renderFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_CLAMP);
    for(size_t h=0; h<objects.size(); ++h)
    {
        if(objects[h].type != RenderableType::SOLID)
            continue;
        content->DrawObject(objects[h].objectId, -1, objects[h].model);
    }
    glEnable(GL_DEPTH_CLAMP);
    OpenGLState::BindFramebuffer(0);
}

void OpenGLDepthCamera::LinearizeDepth()
{
    OpenGLState::BindFramebuffer(linearDepthFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderDepthTex_);
    depthCameraOutputShader[0]->Use();
    depthCameraOutputShader[0]->SetUniform("texDepth", TEX_POSTPROCESS1);
    depthCameraOutputShader[0]->SetUniform("rangeInfo", glm::vec4(range_.x, range_.y, range_.x*range_.y, range_.x-range_.y));
    depthCameraOutputShader[0]->SetUniform("noiseSeed", glm::vec3(randDist_(randGen_), randDist_(randGen_), randDist_(randGen_)));
    depthCameraOutputShader[0]->SetUniform("noiseStddev", noiseDepth_);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::BindFramebuffer(0);
}
    
void OpenGLDepthCamera::Depth2LinearRanges()
{
    glm::mat4 proj = GetProjectionMatrix();
    glm::vec4 projInfo(
                       2.0f/proj[0].x,
                       2.0f/proj[1].y,
                       -(1.f-proj[0].z)/proj[0].x,
                       -(1.f+proj[1].z)/proj[1].y
                       );
    
    OpenGLState::BindFramebuffer(linearDepthFBO_);
    OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderDepthTex_);
    depthCameraOutputShader[1]->Use();
    depthCameraOutputShader[1]->SetUniform("projInfo", projInfo);
    depthCameraOutputShader[1]->SetUniform("rangeInfo", glm::vec4(range_.x, range_.y, range_.x*range_.y, range_.x-range_.y));
    depthCameraOutputShader[1]->SetUniform("texDepth", TEX_POSTPROCESS1);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::BindFramebuffer(0);
}

void OpenGLDepthCamera::DrawLDR(GLuint destinationFBO, bool updated)
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
        if(usesRanges_) Depth2LinearRanges();
        else LinearizeDepth();
        
        int windowHeight = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowHeight();
        
        //Bind depth texture
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex_);
        //LDR drawing
        OpenGLState::BindFramebuffer(destinationFBO);
        OpenGLState::Viewport(dispX, windowHeight-viewportHeight_*dispScale-dispY, viewportWidth_*dispScale, viewportHeight_*dispScale);
        depthVisualizeShader->Use();
        depthVisualizeShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        depthVisualizeShader->SetUniform("range", range_);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
        OpenGLState::BindFramebuffer(0);
        OpenGLState::UseProgram(0);
        //Unbind textures
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    }
    
    //Copy texture to camera buffer
    if(camera_ != nullptr && updated)
    {
        if(!display)
        {
            if(usesRanges_) Depth2LinearRanges();
            else LinearizeDepth();
        }
                
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex_);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, linearDepthPBO_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData_ = true;
    }
}

///////////////////////// Static /////////////////////////////
void OpenGLDepthCamera::Init()
{
    depthCameraOutputShader = new GLSLShader*[2];
    depthCameraOutputShader[0] = new GLSLShader("depthCameraOutput.frag");
    depthCameraOutputShader[0]->AddUniform("rangeInfo", ParameterType::VEC4);
    depthCameraOutputShader[0]->AddUniform("texDepth", ParameterType::INT);
    depthCameraOutputShader[0]->AddUniform("noiseSeed", ParameterType::VEC3);
    depthCameraOutputShader[0]->AddUniform("noiseStddev", ParameterType::FLOAT);
    
    depthCameraOutputShader[1] = new GLSLShader("depthCameraOutput2.frag");
    depthCameraOutputShader[1]->AddUniform("projInfo", ParameterType::VEC4);
    depthCameraOutputShader[1]->AddUniform("rangeInfo", ParameterType::VEC4);
    depthCameraOutputShader[1]->AddUniform("texDepth", ParameterType::INT);
    
    depthVisualizeShader = new GLSLShader("depthVisualize.frag");
    depthVisualizeShader->AddUniform("range", ParameterType::VEC2);
    depthVisualizeShader->AddUniform("texLinearDepth", ParameterType::INT);
}

void OpenGLDepthCamera::Destroy()
{
    if(depthCameraOutputShader != nullptr) 
    {
        if(depthCameraOutputShader[0] != nullptr) delete depthCameraOutputShader[0];
        if(depthCameraOutputShader[1] != nullptr) delete depthCameraOutputShader[1];
        delete [] depthCameraOutputShader;
    }
    if(depthVisualizeShader != nullptr) delete depthVisualizeShader;
}

}
