//
//  OpenGLDepthCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDepthCamera.h"

#include "graphics/Console.h"
#include "sensors/vision/DepthCamera.h"

using namespace sf;

GLSLShader* OpenGLDepthCamera::depthLinearizeShader = NULL;
GLSLShader* OpenGLDepthCamera::depthVisualizeShader = NULL;

OpenGLDepthCamera::OpenGLDepthCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp, GLint originX, GLint originY, GLint width, GLint height, GLfloat fovH, GLfloat minDepth, GLfloat maxDepth) 
 : OpenGLView(originX, originY, width, height)
{
    _needsUpdate = false;
    update = false;
    camera = NULL;
    range.x = UnitSystem::SetLength(minDepth);
    range.y = UnitSystem::SetLength(maxDepth);
    
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();
    
    GLfloat fovx = fovH/180.f*M_PI;
    projection = glm::perspectiveFov(fovx, (GLfloat)viewportWidth, (GLfloat)viewportHeight, range.x, range.y);
    
    //Render depth
    glGenTextures(1, &renderDepthTex);
	glBindTexture(GL_TEXTURE_2D, renderDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, viewportWidth, viewportHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &renderFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderDepthTex, 0);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		cError("Depth Render FBO initialization failed!");
    
    //Linear depth
	glGenTextures(1, &linearDepthTex);
	glBindTexture(GL_TEXTURE_2D, linearDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth, viewportHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture (GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &linearDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, linearDepthFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, linearDepthTex, 0);
		
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		cError("Linear Depth FBO initialization failed!");
}

OpenGLDepthCamera::~OpenGLDepthCamera()
{
    glDeleteTextures(1, &renderDepthTex);
	glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(1, &linearDepthTex);
    glDeleteFramebuffers(1, &linearDepthFBO);
}

void OpenGLDepthCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
	tempDir = _dir;
	tempEye = _eye;
	tempUp = _up;
}

void OpenGLDepthCamera::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupCamera();
}

void OpenGLDepthCamera::SetupCamera()
{
	cameraTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLDepthCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLDepthCamera::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLDepthCamera::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLDepthCamera::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLDepthCamera::GetViewMatrix() const
{
    return cameraTransform;
}

void OpenGLDepthCamera::Update()
{
    _needsUpdate = true;
}

bool OpenGLDepthCamera::needsUpdate()
{
    update = _needsUpdate;
    _needsUpdate = false;
    return update && enabled;
}

void OpenGLDepthCamera::setCamera(DepthCamera* cam)
{
    camera = cam;
}

ViewType OpenGLDepthCamera::getType()
{
    return DEPTH_CAMERA;
}

void OpenGLDepthCamera::LinearizeDepth()
{
    glBindFramebuffer(GL_FRAMEBUFFER, linearDepthFBO);
    glViewport(0, 0, viewportWidth, viewportHeight);
    OpenGLContent::getInstance()->BindBaseVertexArray();
    depthLinearizeShader->Use();
	depthLinearizeShader->SetUniform("clipInfo", glm::vec4(range.x*range.y, range.x-range.y, range.y, 1.f));
	depthLinearizeShader->SetUniform("texDepth", TEX_POSTPROCESS1);
		
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, renderDepthTex);
	glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindTexture(GL_TEXTURE_2D, 0);
	
    glUseProgram(0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLDepthCamera::DrawLDR(GLuint destinationFBO)
{
    //Check if there is a need to display image on screen
    bool display = true;
    if(camera != NULL)
        display = camera->getDisplayOnScreen();
    
    //Draw on screen
    if(display)
    {
        LinearizeDepth();
        
        //Bind depth texture
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
        glBindTexture(GL_TEXTURE_2D, linearDepthTex);
       
        //LDR drawing
        glBindFramebuffer(GL_FRAMEBUFFER, destinationFBO);
        glViewport(0, 0, viewportWidth, viewportHeight);
        depthVisualizeShader->Use();
        depthVisualizeShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        depthVisualizeShader->SetUniform("range", range);
        OpenGLContent::getInstance()->DrawSAQ();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
        
        //Unbind textures
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    //Copy texture to camera buffer
    if(camera != NULL && update)
    {
        if(!display)
            LinearizeDepth();
        
        glBindTexture(GL_TEXTURE_2D, linearDepthTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, camera->getDataPointer());
        glBindTexture(GL_TEXTURE_2D, 0);
         
        //Inform camera to run callback
        camera->NewDataReady();
    }
    
    update = false;
}

///////////////////////// Static /////////////////////////////
void OpenGLDepthCamera::Init()
{
	depthLinearizeShader = new GLSLShader("depthLinearize.frag");
	depthLinearizeShader->AddUniform("clipInfo", ParameterType::VEC4);
	depthLinearizeShader->AddUniform("texDepth", ParameterType::INT);
    
    depthVisualizeShader = new GLSLShader("depthVisualize.frag");
    depthVisualizeShader->AddUniform("texLinearDepth", ParameterType::INT);
    depthVisualizeShader->AddUniform("range", ParameterType::VEC2);
}

void OpenGLDepthCamera::Destroy()
{
    if(depthLinearizeShader != NULL) delete depthLinearizeShader;
    if(depthVisualizeShader != NULL) delete depthVisualizeShader;
}
