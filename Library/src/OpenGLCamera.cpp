//
//  OpenGLCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLCamera.h"
#include "Camera.h"
#include "Console.h"
#include "MathsUtil.hpp"

OpenGLCamera::OpenGLCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint x, GLint y, GLint width, GLint height, GLfloat fovH, GLfloat horizon, GLuint spp, bool sao) : OpenGLView(x, y, width, height, horizon, spp, sao)
{
    _needsUpdate = false;
    update = false;
    camera = NULL;
    cameraFBO = 0;
    cameraColorTex = 0;
    
    //Setup view
    pan = 0.f;
    tilt = 0.f;
    SetupCamera(eyePosition, direction, cameraUp);

    //Setup projection
    fovx = fovH/180.f*M_PI;
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = fovx/aspect;
    projection = glm::perspective(fovy, aspect, near, far);
}

OpenGLCamera::~OpenGLCamera()
{
    if(camera != NULL)
    {
        glDeleteFramebuffers(1, &cameraFBO);
        glDeleteTextures(1, &cameraColorTex);
    }
}

ViewType OpenGLCamera::getType()
{
    return CAMERA;
}

void OpenGLCamera::RotateCamera(btScalar panStep, btScalar tiltStep)
{
    pan += UnitSystem::SetAngle(panStep);
    tilt += UnitSystem::SetAngle(tiltStep);
    SetupCamera();
}

void OpenGLCamera::Update()
{
    _needsUpdate = true;
}

void OpenGLCamera::setRendering(bool render)
{
	if(!rendering && render) //Rendering started
	{
		dir = tempDir;
		eye = tempEye;
		up = tempUp;
		SetupCamera();
	}
   
	rendering = render;
}

bool OpenGLCamera::needsUpdate()
{
    update = _needsUpdate;
    _needsUpdate = false;
    return update && enabled;
}

void OpenGLCamera::setCamera(Camera* cam)
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

void OpenGLCamera::setPanAngle(GLfloat newPanAngle)
{
    newPanAngle = UnitSystem::SetAngle(newPanAngle);
    pan = newPanAngle;
    SetupCamera();
}

void OpenGLCamera::setTiltAngle(GLfloat newTiltAngle)
{
    newTiltAngle = UnitSystem::SetAngle(newTiltAngle);
    tilt = newTiltAngle;
    SetupCamera();
}

GLfloat OpenGLCamera::getPanAngle()
{
    return UnitSystem::GetAngle(pan);
}

GLfloat OpenGLCamera::getTiltAngle()
{
    return UnitSystem::GetAngle(tilt);
}

glm::vec3 OpenGLCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLCamera::GetLookingDirection() const
{
    return lookingDir;
}

glm::vec3 OpenGLCamera::GetUpDirection() const
{
    return currentUp;
}

void OpenGLCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    if(!rendering)
	{
		dir = _dir;
		eye = _eye;
		up = _up;
		SetupCamera();
	}
	
	tempDir = _dir;
	tempEye = _eye;
	tempUp = _up;
}

void OpenGLCamera::SetupCamera()
{
    lookingDir = dir;
    currentUp = up;
    
    //additional camera rotation
    glm::vec3 tiltAxis = glm::normalize(glm::cross(dir, up));
    glm::vec3 panAxis = glm::normalize(glm::cross(tiltAxis, dir));
    
    //rotate
	lookingDir = glm::rotate(lookingDir, tilt, tiltAxis);
	lookingDir = glm::rotate(lookingDir, pan, panAxis);
    lookingDir = glm::normalize(lookingDir);
    
    currentUp = glm::rotate(currentUp, tilt, tiltAxis);
    currentUp = glm::rotate(currentUp, pan, panAxis);
	currentUp = glm::normalize(currentUp);
    
	cameraTransform = glm::lookAt(eye, eye+lookingDir, currentUp);
	cameraRender = glm::inverse(cameraTransform);
}

glm::mat4 OpenGLCamera::GetViewTransform() const
{
    return cameraTransform;
}

void OpenGLCamera::DrawHDR(GLuint destinationFBO)
{
    //Draw to screen
    OpenGLView::DrawHDR(destinationFBO);
    
    //Draw to camera buffer (no need to calculate exposure again)
    if(camera != NULL && update)
    {
        //Draw LDR
        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, postprocessTex[0]);
        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, lightMeterTex);
        
        glBindFramebuffer(GL_FRAMEBUFFER, cameraFBO);
        glViewport(0, 0, viewportWidth, viewportHeight);
        tonemapShader->Use();
        tonemapShader->SetUniform("texHDR", TEX_POSTPROCESS1);
        tonemapShader->SetUniform("texAverage", TEX_POSTPROCESS2);
        OpenGLContent::getInstance()->DrawSAQ();
        glUseProgram(0);
        
        //Copy to camera data
        glReadPixels(0, 0, viewportWidth, viewportHeight, GL_RGB, GL_UNSIGNED_BYTE, camera->getDataPointer());
        
        //Unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
        glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
        
        //Inform camera to run callback
        camera->NewDataReady();
    }
    
    update = false;
}