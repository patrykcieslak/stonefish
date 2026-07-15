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
//  OpenGLEventBasedCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/03/24.
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLEventBasedCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "sensors/vision/EventBasedCamera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

std::array<std::unique_ptr<GLSLShader>, 2> OpenGLEventBasedCamera::eventOutputShaders;
std::unique_ptr<GLSLShader> OpenGLEventBasedCamera::eventVisualizeShader;

OpenGLEventBasedCamera::OpenGLEventBasedCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                                   GLint x, GLint y, GLint width, GLint height, GLfloat horizontalFovDeg, 
                                   glm::vec2 range, glm::vec2 C, uint32_t Tr, bool continuousUpdate) 
                                   : OpenGLCamera(x, y, width, height, range), randDist_(0.f, 1.f)
{  
    needsUpdate_ = false;
    newData_ = false;
    initialized_ = false;
    lastSimTime_ = -1.f;
    continuous_ = continuousUpdate;
    camera_ = nullptr;
    C_ = C;
    Tr_ = Tr;
    sigmaC_ = glm::vec2(0.f);
    
    // Allocate necessary textures
    // --> Logarithm of luminance
    renderLogLumTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                     GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    // --> Events (R -> pos x | pos y in px, G -> timestamp in ns, with polarization)
    maxNumEvents_ = viewportWidth_ * viewportHeight_ * 10;
    glm::uvec3 maxTextureSize = OpenGLState::GetMaxTextureSize();
    glm::uvec3 eventsTextureSize;
    if(maxNumEvents_ > maxTextureSize.x)
    {
        eventsTextureSize.x = maxTextureSize.x;
        eventsTextureSize.y = maxNumEvents_/maxTextureSize.x + 1;
        eventsTextureSize.z = 0;
        maxNumEvents_ = eventsTextureSize.x * eventsTextureSize.y;
    }
    else
    {
        eventsTextureSize.x = maxNumEvents_;
        eventsTextureSize.y = 1;
        eventsTextureSize.z = 0;
    }
    renderEventTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, eventsTextureSize, 
                                                     GL_RG32I, GL_RG_INTEGER, GL_INT, NULL, FilteringMode::NEAREST, false);
    // --> Last event timestamp (in ns, with polarization)
    renderEventTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                     GL_R32I, GL_RED_INTEGER, GL_INT, NULL, FilteringMode::NEAREST, false);
    // --> Crossings
    renderEventTex_[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                     GL_R32F, GL_RED, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    // Allocate buffer to store the event counter
    GLuint zero = 0;
    glGenBuffers(1, &renderEventCounter_);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, renderEventCounter_); 
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_STREAM_READ);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // Event display
    displayTex_ = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth_, viewportHeight_, 0), 
                                                     GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);
    std::vector<FBOTexture> fboTextures;
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayTex_));
    displayFBO_ = OpenGLContent::GenerateFramebuffer(fboTextures);
                                                     
    // Setup view
    SetupCamera(eyePosition, direction, cameraUp);
    // Setup projection
    fovx_ = horizontalFovDeg/180.f * M_PI;
    GLfloat fovy = 2.f * atanf( (GLfloat)viewportHeight_/(GLfloat)viewportWidth_ * tanf(fovx_/2.f) );
    projection_ = glm::perspectiveFov(fovy, (GLfloat)viewportWidth_, (GLfloat)viewportHeight_, near_, far_);

    UpdateTransform();
}

OpenGLEventBasedCamera::~OpenGLEventBasedCamera()
{
    glDeleteTextures(1, &renderLogLumTex_);
    glDeleteTextures(3, renderEventTex_);
    glDeleteBuffers(1, &renderEventCounter_);
    glDeleteFramebuffers(1, &displayFBO_);
    glDeleteTextures(1, &displayTex_);

    if(camera_ != nullptr)
        glDeleteBuffers(1, &outputPBO_);    
}

ViewType OpenGLEventBasedCamera::getType() const
{
    return ViewType::EVENT_BASED_CAMERA;
}

void OpenGLEventBasedCamera::Update()
{
    needsUpdate_ = true;
}

bool OpenGLEventBasedCamera::needsUpdate()
{
    if(needsUpdate_)
    {
        needsUpdate_ = false;
        return enabled_;
    }
    else
        return false;
}

void OpenGLEventBasedCamera::setCamera(EventBasedCamera* cam)
{
    //Connect with camera sensor
    camera_ = cam;    
    glGenBuffers(1, &outputPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, maxNumEvents_ * 2 * sizeof(GLint), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLEventBasedCamera::setNoise(glm::vec2 sigmaC)
{
    sigmaC_ = sigmaC;
}

glm::vec3 OpenGLEventBasedCamera::GetEyePosition() const
{
    return eye_;
}

glm::vec3 OpenGLEventBasedCamera::GetLookingDirection() const
{
    return dir_;
}

glm::vec3 OpenGLEventBasedCamera::GetUpDirection() const
{
    return up_;
}

void OpenGLEventBasedCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir_ = _dir;
    tempEye_ = _eye;
    tempUp_ = _up;
}

void OpenGLEventBasedCamera::UpdateTransform()
{
    eye_ = tempEye_;
    dir_ = tempDir_;
    up_ = tempUp_;
    SetupCamera();
    
    viewUBOData_.VP = GetProjectionMatrix() * GetViewMatrix();
    viewUBOData_.eye = GetEyePosition();
    ExtractFrustumFromVP(viewUBOData_.frustum, viewUBOData_.VP);

    //Inform camera to run callback
    if(newData_)
    {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, renderEventCounter_);
        GLuint* pEventCounter = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
        GLuint lastEventCount = *pEventCounter;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
        GLint* src = (GLint*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera_->NewDataReady(src, lastEventCount);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData_ = false;
    }
}

void OpenGLEventBasedCamera::SetupCamera()
{
    cameraTransform_ = glm::lookAt(eye_, eye_+dir_, up_);
}

glm::mat4 OpenGLEventBasedCamera::GetViewMatrix() const
{
    return cameraTransform_;
}

void OpenGLEventBasedCamera::ComputeOutput(double simTime)
{
    if(initialized_)
    {
        // Reset event counter
        GLuint zero = 0;
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, renderEventCounter_);
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        // Compute events
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, renderEventCounter_);
        glBindImageTexture(TEX_POSTPROCESS1, renderColorTex_[lastActiveRenderColorBuffer_], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(TEX_POSTPROCESS2, renderLogLumTex_, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        glBindImageTexture(TEX_POSTPROCESS3, renderEventTex_[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32I);
        glBindImageTexture(TEX_POSTPROCESS4, renderEventTex_[1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
        glBindImageTexture(TEX_POSTPROCESS5, renderEventTex_[2], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        eventOutputShaders[1]->Use();
        eventOutputShaders[1]->SetUniform("currColor", TEX_POSTPROCESS1);
        eventOutputShaders[1]->SetUniform("logLum", TEX_POSTPROCESS2);
        eventOutputShaders[1]->SetUniform("events", TEX_POSTPROCESS3);
        eventOutputShaders[1]->SetUniform("eventTimes", TEX_POSTPROCESS4);
        eventOutputShaders[1]->SetUniform("crossings", TEX_POSTPROCESS5);
        Scalar dt = simTime - lastSimTime_;
        eventOutputShaders[1]->SetUniform("dT", (int32_t)round(dt*1e9));
        eventOutputShaders[1]->SetUniform("Tr", Tr_);
        eventOutputShaders[1]->SetUniform("C", C_);
        eventOutputShaders[1]->SetUniform("sigmaC", sigmaC_);
        eventOutputShaders[1]->SetUniform("seed", glm::vec3(randDist_(randGen_), randDist_(randGen_), randDist_(randGen_)));
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
        glDispatchCompute((GLuint)ceilf(viewportWidth_/16.f), (GLuint)ceilf(viewportHeight_/16.f), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    }
    else
    {
        // Compute logarithm of luminance of the first frame (initialization)
        glBindImageTexture(TEX_POSTPROCESS1, renderColorTex_[lastActiveRenderColorBuffer_], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(TEX_POSTPROCESS2, renderLogLumTex_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glBindImageTexture(TEX_POSTPROCESS4, renderEventTex_[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);
        glBindImageTexture(TEX_POSTPROCESS5, renderEventTex_[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        
        eventOutputShaders[0]->Use();
        eventOutputShaders[0]->SetUniform("currColor", TEX_POSTPROCESS1);
        eventOutputShaders[0]->SetUniform("logLum", TEX_POSTPROCESS2);
        eventOutputShaders[0]->SetUniform("eventTimes", TEX_POSTPROCESS4);
        eventOutputShaders[0]->SetUniform("crossings", TEX_POSTPROCESS5);
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
        glDispatchCompute((GLuint)ceilf(viewportWidth_/16.f), (GLuint)ceilf(viewportHeight_/16.f), 1); 
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        initialized_ = true;
    }
    lastSimTime_ = simTime;
}

void OpenGLEventBasedCamera::DrawLDR(GLuint destinationFBO, bool updated)
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
        // Draw color image
        OpenGLCamera::DrawLDR(displayFBO_, updated);

        // Overlay event information
        OpenGLContent* content = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent();
        int windowHeight = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getWindowHeight();
        int windowWidth = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getWindowWidth();
        
        glm::vec4 rect((GLfloat)dispX, (GLfloat)dispY, viewportWidth_*dispScale, viewportHeight_*dispScale);
        rect.y = windowHeight-rect.y-rect.w;
        rect.x /= windowWidth;
        rect.y /= windowHeight;
        rect.z /= windowWidth;
        rect.w /= windowHeight;

        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, displayTex_);
        OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, renderEventTex_[1]);
        
        OpenGLState::BindFramebuffer(destinationFBO);
        content->SetViewportSize(windowWidth, windowHeight);
        OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
        OpenGLState::DisableCullFace();
        eventVisualizeShader->Use();
        eventVisualizeShader->SetUniform("rect", rect);
        eventVisualizeShader->SetUniform("texCamera", TEX_POSTPROCESS1);
        eventVisualizeShader->SetUniform("texEventTimes", TEX_POSTPROCESS2);        
        content->BindBaseVertexArray();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        OpenGLState::BindVertexArray(0);
        OpenGLState::UseProgram(0);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);

        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    }
    
    //Copy data to camera buffer
    if(camera_ != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderEventTex_[0]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RG_INTEGER, GL_INT, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData_ = true;
    }
}

void OpenGLEventBasedCamera::Init()
{
    glm::uvec3 maxTextureSize = OpenGLState::GetMaxTextureSize();

    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "eventInit.comp"));
    eventOutputShaders[0] = std::make_unique<GLSLShader>(sources);
    eventOutputShaders[0]->AddUniform("currColor", ParameterType::INT);
    eventOutputShaders[0]->AddUniform("logLum", ParameterType::INT);
    eventOutputShaders[0]->AddUniform("eventTimes", ParameterType::INT);
    eventOutputShaders[0]->AddUniform("crossings", ParameterType::INT);
    
    sources.clear();
    std::string header = "#version 430\n#define MAX_TEXTURE_WIDTH " + std::to_string(maxTextureSize.x) + "\n";
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "event.comp", header));
    eventOutputShaders[1] = std::make_unique<GLSLShader>(sources);
    eventOutputShaders[1]->AddUniform("currColor", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("logLum", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("events", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("eventTimes", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("crossings", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("dT", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("Tr", ParameterType::INT);
    eventOutputShaders[1]->AddUniform("C", ParameterType::VEC2);
    eventOutputShaders[1]->AddUniform("sigmaC", ParameterType::VEC2);
    eventOutputShaders[1]->AddUniform("seed", ParameterType::VEC3);


    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "texQuad.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "eventVisualize.frag"));
    eventVisualizeShader = std::make_unique<GLSLShader>(sources);
    eventVisualizeShader->AddUniform("rect", ParameterType::VEC4);
    eventVisualizeShader->AddUniform("texCamera", ParameterType::INT);
    eventVisualizeShader->AddUniform("texEventTimes", ParameterType::INT);
}
        
void OpenGLEventBasedCamera::Destroy()
{
    eventOutputShaders[0].reset();
    eventOutputShaders[1].reset();
    eventVisualizeShader.reset();
}

}