//
//  OpenGLRealCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLRealCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "core/Console.h"
#include "sensors/vision/ColorCamera.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

OpenGLRealCamera::OpenGLRealCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                                   GLint x, GLint y, GLint width, GLint height,
                                   GLfloat horizontalFovDeg, GLfloat horizonDistance, GLuint spp, bool sao) : OpenGLCamera(x, y, width, height, horizonDistance, spp, sao)
{
    _needsUpdate = false;
    update = false;
    camera = NULL;
    cameraFBO = 0;
    cameraColorTex = 0;
    
    //Setup view
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();

    //Setup projection
    fovx = horizontalFovDeg/180.f*M_PI;
    projection = glm::perspectiveFov(fovx*(GLfloat)viewportHeight/(GLfloat)viewportWidth, (GLfloat)viewportWidth, (GLfloat)viewportHeight, near, far);
}

OpenGLRealCamera::~OpenGLRealCamera()
{
    if(camera != NULL)
    {
        glDeleteFramebuffers(1, &cameraFBO);
        glDeleteTextures(1, &cameraColorTex);
    }
}

ViewType OpenGLRealCamera::getType()
{
    return CAMERA;
}

void OpenGLRealCamera::Update()
{
    _needsUpdate = true;
}

bool OpenGLRealCamera::needsUpdate()
{
    update = _needsUpdate;
    _needsUpdate = false;
    return update && enabled;
}

void OpenGLRealCamera::setCamera(ColorCamera* cam)
{
    //Connect with camera sensor
    camera = cam;
    
    //Generate buffers
    glGenFramebuffers(1, &cameraFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, cameraFBO);
    
    glGenTextures(1, &cameraColorTex);
    glBindTexture(GL_TEXTURE_2D, cameraColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cameraColorTex, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Camera FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
}

void OpenGLRealCamera::SetupCamera()
{
	cameraTransform = glm::lookAt(eye, eye+dir, up);
}

glm::mat4 OpenGLRealCamera::GetViewMatrix() const
{
    return cameraTransform;
}

void OpenGLRealCamera::DrawLDR(GLuint destinationFBO)
{
    //Check if there is a need to display image on screen
    bool display = true;
    if(camera != NULL)
        display = camera->getDisplayOnScreen();
    
    //Draw on screen
    if(display)
        OpenGLCamera::DrawLDR(destinationFBO);
    
    //Draw to camera buffer
    if(camera != NULL && update)
    {
        if(display) //No need to calculate exposure again
        {
            glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
            glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
            glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
            glBindTexture(GL_TEXTURE_2D, lightMeterTex);
            
            glBindFramebuffer(GL_FRAMEBUFFER, cameraFBO);
            //glViewport(0, 0, viewportWidth, viewportHeight);
            tonemapShader->Use();
            tonemapShader->SetUniform("texHDR", TEX_POSTPROCESS1);
            tonemapShader->SetUniform("texAverage", TEX_POSTPROCESS2);
            tonemapShader->SetUniform("exposureComp", 1.f);
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
            glUseProgram(0);
        
            //Copy to camera data
            glReadPixels(0, 0, viewportWidth, viewportHeight, GL_RGB, GL_UNSIGNED_BYTE, camera->getImageDataPointer());
        
            //Unbind
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            OpenGLCamera::DrawLDR(cameraFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, cameraFBO);
            glReadPixels(0, 0, viewportWidth, viewportHeight, GL_RGB, GL_UNSIGNED_BYTE, camera->getImageDataPointer());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        //Inform camera to run callback
        camera->NewDataReady();
    }
    
    update = false;
}

}
