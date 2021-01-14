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
//  Copyright (c) 2012-2020 Patryk Cieslak. All rights reserved.
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
    _needsUpdate = false;
    newData = false;
    continuous = continuousUpdate;
    camera = NULL;
    cameraFBO = 0;
    
    //Setup view
    SetupCamera(eyePosition, direction, cameraUp);
    //Setup projection
    fovx = horizontalFovDeg/180.f * M_PI;
    GLfloat fovy = 2.f * atanf( (GLfloat)viewportHeight/(GLfloat)viewportWidth * tanf(fovx/2.f) );
    projection = glm::perspectiveFov(fovy, (GLfloat)viewportWidth, (GLfloat)viewportHeight, near, far);

    UpdateTransform();
}

OpenGLRealCamera::~OpenGLRealCamera()
{
    if(camera != NULL)
    {
        glDeleteFramebuffers(1, &cameraFBO);
        glDeleteBuffers(1, &cameraPBO);
        glDeleteTextures(2, cameraColorTex);
    }
}

ViewType OpenGLRealCamera::getType()
{
    return ViewType::CAMERA;
}

void OpenGLRealCamera::Update()
{
    _needsUpdate = true;
}

bool OpenGLRealCamera::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLRealCamera::setCamera(ColorCamera* cam)
{
    //Connect with camera sensor
    camera = cam;
    
    //Generate buffers
    cameraColorTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3((GLuint)viewportWidth, (GLuint)viewportHeight, 0), 
                                                       GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, sf::FilteringMode::NEAREST, false);
    cameraColorTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3((GLuint)viewportWidth, (GLuint)viewportHeight, 0), 
                                                       GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, sf::FilteringMode::NEAREST, false);
    std::vector<FBOTexture> textures;
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cameraColorTex[0]));
    textures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, cameraColorTex[1]));
    cameraFBO = OpenGLContent::GenerateFramebuffer(textures);
    
    glGenBuffers(1, &cameraPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

glm::vec3 OpenGLRealCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLRealCamera::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLRealCamera::GetUpDirection() const
{
    return up;
}

void OpenGLRealCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempEye = _eye;
    tempUp = _up;
}

void OpenGLRealCamera::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupCamera();
    
    viewUBOData.VP = GetProjectionMatrix() * GetViewMatrix();
    viewUBOData.eye = GetEyePosition();
    ExtractFrustumFromVP(viewUBOData.frustum, viewUBOData.VP);

    //Inform camera to run callback
    if(newData)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera->NewDataReady(src);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData = false;
    }
}

void OpenGLRealCamera::SetupCamera()
{
    cameraTransform = glm::lookAt(eye, eye+dir, up);
}

glm::mat4 OpenGLRealCamera::GetViewMatrix() const
{
    return cameraTransform;
}

void OpenGLRealCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
    //Check if there is a need to display image on screen
    bool display = true;
    unsigned int dispX, dispY;
    GLfloat dispScale;
    if(camera != NULL)
        display = camera->getDisplayOnScreen(dispX, dispY, dispScale);
    
    //Draw on screen
    if(display)
        OpenGLCamera::DrawLDR(destinationFBO, updated);
    
    //Draw to camera buffer
    if(camera != NULL && updated)
    {
        if(display) //No need to calculate exposure again
        {
            if(antiAliasing)
            {
                OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, postprocessTex[1]);
                OpenGLState::BindFramebuffer(cameraFBO);
                OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                fxaaShader->Use();
                fxaaShader->SetUniform("texSource", TEX_POSTPROCESS1);
                fxaaShader->SetUniform("RCPFrame", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight));
                ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
                
                glDrawBuffer(GL_COLOR_ATTACHMENT1);
                OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex[0]);
                flipShader->Use();
                flipShader->SetUniform("texSource", TEX_POSTPROCESS1);
                ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
                OpenGLState::UseProgram(0);
                OpenGLState::BindFramebuffer(0);

                OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex[1]);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
            }
            else
            {
                OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderColorTex[lastActiveRenderColorBuffer]);
			    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, exposureTex);
                OpenGLState::BindFramebuffer(cameraFBO);
                OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                tonemappingShaders[2]->Use();
                tonemappingShaders[2]->SetUniform("texSource", TEX_POSTPROCESS1);
                tonemappingShaders[2]->SetUniform("texExposure", TEX_POSTPROCESS2);
                tonemappingShaders[2]->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp));
                ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
                OpenGLState::UnbindTexture(TEX_POSTPROCESS2);

                glDrawBuffer(GL_COLOR_ATTACHMENT1);
                OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex[0]);
                flipShader->Use();
                flipShader->SetUniform("texSource", TEX_POSTPROCESS1);
                ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
                OpenGLState::UseProgram(0);
                OpenGLState::BindFramebuffer(0);

                OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex[1]);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
            }
        }
        else
        {
            OpenGLCamera::DrawLDR(cameraFBO, updated);
            OpenGLState::BindFramebuffer(cameraFBO);
            OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex[0]);
            flipShader->Use();
            flipShader->SetUniform("texSource", TEX_POSTPROCESS1);
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            OpenGLState::UseProgram(0);
            OpenGLState::BindFramebuffer(0);

            OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cameraColorTex[1]);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, cameraPBO);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        }
        
        newData = true;
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