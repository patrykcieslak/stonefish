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
//  OpenGLRealCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLRealCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "sensors/vision/ColorCamera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

OpenGLRealCamera::OpenGLRealCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                                   GLint x, GLint y, GLint width, GLint height,
                                   GLfloat horizontalFovDeg, glm::vec2 range, bool continuousUpdate) 
                                   : OpenGLCamera(x, y, width, height, range)
{
    needsUpdate_ = false;
    newData_ = false;
    continuous_ = continuousUpdate;
    camera_ = nullptr;
    cameraFBO_ = 0;
    
    //Setup view
    SetupCamera(eyePosition, direction, cameraUp);
    //Setup projection
    fovx_ = horizontalFovDeg/180.f * M_PI;
    GLfloat fovy = 2.f * atanf( (GLfloat)viewportHeight_/(GLfloat)viewportWidth_ * tanf(fovx_/2.f) );
    projection_ = glm::perspectiveFov(fovy, (GLfloat)viewportWidth_, (GLfloat)viewportHeight_, near_, far_);

    UpdateTransform();
}

OpenGLRealCamera::~OpenGLRealCamera()
{
    if(camera_ != nullptr)
    {
        glDeleteFramebuffers(1, &cameraFBO_);
        glDeleteBuffers(1, &cameraPBO_);
        glDeleteTextures(2, cameraColorTex_);
    }
}

ViewType OpenGLRealCamera::getType() const
{
    return ViewType::CAMERA;
}

void OpenGLRealCamera::Update()
{
    needsUpdate_ = true;
}

bool OpenGLRealCamera::needsUpdate()
{
    if(needsUpdate_)
    {
        needsUpdate_ = false;
        return enabled_;
    }
    else
        return false;
}

void OpenGLRealCamera::setCamera(ColorCamera* cam)
{
    //Connect with camera sensor
    camera_ = cam;
    
    //Generate buffers
    cameraColorTex_[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3((GLuint)viewportWidth_, (GLuint)viewportHeight_, 0), 
                                                       GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, sf::FilteringMode::NEAREST, false);
    cameraColorTex_[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3((GLuint)viewportWidth_, (GLuint)viewportHeight_, 0), 
                                                       GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, sf::FilteringMode::NEAREST, false);
    std::vector<FBOTexture> textures;
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cameraColorTex_[0]));
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, cameraColorTex_[1]));
    cameraFBO_ = OpenGLContent::GenerateFramebuffer(textures);
    
    glGenBuffers(1, &cameraPBO_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO_);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth_ * viewportHeight_ * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

glm::vec3 OpenGLRealCamera::GetEyePosition() const
{
    return eye_;
}

glm::vec3 OpenGLRealCamera::GetLookingDirection() const
{
    return dir_;
}

glm::vec3 OpenGLRealCamera::GetUpDirection() const
{
    return up_;
}

void OpenGLRealCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir_ = _dir;
    tempEye_ = _eye;
    tempUp_ = _up;
}

void OpenGLRealCamera::UpdateTransform()
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
        glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO_);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera_->NewDataReady(src);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData_ = false;
    }
}

void OpenGLRealCamera::SetupCamera()
{
    cameraTransform_ = glm::lookAt(eye_, eye_+dir_, up_);
}

glm::mat4 OpenGLRealCamera::GetViewMatrix() const
{
    return cameraTransform_;
}

void OpenGLRealCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
    if(camera_ != nullptr && updated)
    {
        OpenGLCamera::DrawLDR(cameraFBO_, updated);

        OpenGLState::BindFramebuffer(cameraFBO_);
        OpenGLState::Viewport(0, 0, viewportWidth_, viewportHeight_);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex_[0]);
        shaders["flip"]->Use();
        shaders["flip"]->SetUniform("texSource", TEX_POSTPROCESS1);
        static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
        OpenGLState::UseProgram(0);
        OpenGLState::BindFramebuffer(0);

        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex_[1]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData_ = true;
    }
    
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
        OpenGLState::DisableCullFace();
        OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
        content->SetViewportSize(windowWidth, windowHeight);
        content->DrawTexturedQuad(dispX, dispY, viewportWidth_*dispScale, viewportHeight_*dispScale, cameraColorTex_[0]);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);
    }
}

}

/*
float computeEV100(float aperture, float shutterTime , float ISO)
{
    //EV number is defined as:
    // 2^ EV_s = N ^2 / t    and     EV_s = EV_100 + log2 ( S /100)
    // This gives
    // EV_s = log2 ( N ^2 / t )
    // EV_100 + log2 ( S /100) = log2 ( N ^2 / t )
    // EV_100 = log2 ( N ^2 / t ) - log2 ( S /100)
    // EV_100 = log2 ( N ^2 / t . 100 / S )
    return log2(aperture*aperture/shutterTime*100.0/ISO);
}

float computeEV100FromAvgLuminance(float avgLuminance)
{
    // We later use the middle gray at 12.7% in order to have
    // a middle gray at 18% with a sqrt (2) room for specular highlights
    // But here we deal with the spot meter measuring the middle gray
    // which is fixed at 12.5 for matching standard camera
    // constructor settings ( i . e . calibration constant K = 12.5)
    // Reference: http://en.wikipedia.org/wiki/Film_speed
    return log2(avgLuminance*100.0/12.5);
}

float convertEV100ToExposure(float EV100)
{
    // Compute the maximum luminance possible with H_sbs sensitivity
    // maxLum = 78 / ( S * q ) * N ^2 / t
    //        = 78 / ( S * q ) * 2^ EV_100
    //        = 78 / (100 * 0.65) * 2^ EV_100
    //        = 1.2 * 2^ EV
    // Reference: http://en.wikipedia.org/wiki/Film_speed
    float maxLuminance = 1.2*pow(2.0, EV100);
    return 1.0/maxLuminance;
}*/