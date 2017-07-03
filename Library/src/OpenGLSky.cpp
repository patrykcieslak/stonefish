//
//  OpenGLSky.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 9/18/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSky.h"
#include "GeometryUtil.hpp"
#include "OpenGLSun.h"
#include "Console.h"

OpenGLSky* OpenGLSky::instance = NULL;

OpenGLSky* OpenGLSky::getInstance()
{
    if(instance == NULL)
        instance = new OpenGLSky();
    
    return instance;
}

OpenGLSky::OpenGLSky()
{
    skyCubeFBO = 0;
    skyCubemap = 0;
    skyCubeSize = 512;
    skyCubeShader = NULL;
    
    ds2FBO = 0;
    ds2Cubemap = 0;
    ds4FBO = 0;
    ds4Cubemap = 0;
    ds8FBO = 0;
    ds8Cubemap = 0;
    dsShader = NULL;
    
    convolveFBO = 0;
    convolveDiffuseCubemap = 0;
    convolveReflectionCubemap = 0;
    convolveShader = NULL;
    
    skyDrawShader = NULL;
    debugAngle = 0;
}

OpenGLSky::~OpenGLSky()
{
    glDeleteTextures(1, &skyCubemap);
    glDeleteTextures(1, &ds2Cubemap);
    glDeleteTextures(1, &ds4Cubemap);
    glDeleteTextures(1, &ds8Cubemap);
    glDeleteTextures(1, &convolveDiffuseCubemap);
    glDeleteTextures(1, &convolveReflectionCubemap);
    glDeleteFramebuffers(1, &skyCubeFBO);
    glDeleteFramebuffers(1, &ds2FBO);
    glDeleteFramebuffers(1, &ds4FBO);
    glDeleteFramebuffers(1, &ds8FBO);
    glDeleteFramebuffers(1, &convolveFBO);
    delete skyCubeShader;
    delete convolveShader;
    delete dsShader;
    delete skyDrawShader;
}

void OpenGLSky::Init()
{
    ////////Generate FBOs
    //Sky Cubemap
    glGenFramebuffers(1, &skyCubeFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, skyCubeFBO);
    
    glGenTextures(1, &skyCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, skyCubeSize, skyCubeSize, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, skyCubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sky FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //Downsampling
    //x2
    glGenFramebuffers(1, &ds2FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ds2FBO);
    
    glGenTextures(1, &ds2Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds2Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/2, skyCubeSize/2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ds2Cubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sky downsample x2 FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //x4
    glGenFramebuffers(1, &ds4FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ds4FBO);
    
    glGenTextures(1, &ds4Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds4Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/4, skyCubeSize/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ds4Cubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sky downsample x4 FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //x8
    glGenFramebuffers(1, &ds8FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ds8FBO);
    
    glGenTextures(1, &ds8Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds8Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/8, skyCubeSize/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ds8Cubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sky downsample x8 FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Convolution
    glGenFramebuffers(1, &convolveFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, convolveFBO);
    
    glGenTextures(1, &convolveDiffuseCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, convolveDiffuseCubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/8, skyCubeSize/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, convolveDiffuseCubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glGenTextures(1, &convolveReflectionCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, convolveReflectionCubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/8, skyCubeSize/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X, convolveReflectionCubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Sky convolve FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ////////Load shaders
    //Sky Cubemap
    skyCubeShader = new GLSLShader("cubeSky.frag");
    skyCubeShader->AddUniform("viewport", VEC2);
    skyCubeShader->AddUniform("inv_proj", MAT4);
    skyCubeShader->AddUniform("inv_view_rot", MAT3);
    skyCubeShader->AddUniform("lightdir", VEC3);
    skyCubeShader->AddUniform("Kr", VEC3);
    skyCubeShader->AddUniform("rayleigh_brightness", FLOAT);
    skyCubeShader->AddUniform("mie_brightness", FLOAT);
    skyCubeShader->AddUniform("spot_brightness", FLOAT);
    skyCubeShader->AddUniform("scatter_strength", FLOAT);
    skyCubeShader->AddUniform("rayleigh_strength", FLOAT);
    skyCubeShader->AddUniform("mie_strength", FLOAT);
    skyCubeShader->AddUniform("rayleigh_collection_power", FLOAT);
    skyCubeShader->AddUniform("mie_collection_power", FLOAT);
    skyCubeShader->AddUniform("mie_distribution", FLOAT);
    
    //Downsample
    dsShader = new GLSLShader("cubeDownsample.frag");
    dsShader->AddUniform("viewport", VEC2);
    dsShader->AddUniform("inv_proj", MAT4);
    dsShader->AddUniform("inv_view_rot", MAT3);
    dsShader->AddUniform("source", INT);
    
    //Convolve
    convolveShader = new GLSLShader("cubeConvolve.frag");
    convolveShader->AddUniform("viewport", VEC2);
    convolveShader->AddUniform("inv_proj", MAT4);
    convolveShader->AddUniform("inv_view_rot", MAT3);
    convolveShader->AddUniform("source", INT);
    convolveShader->AddUniform("specularity", FLOAT);
    
    //Draw Sky
    skyDrawShader = new GLSLShader("cubePass.frag");
    skyDrawShader->AddUniform("viewport", VEC2);
    skyDrawShader->AddUniform("inv_proj", MAT4);
    skyDrawShader->AddUniform("inv_view_rot", MAT3);
    skyDrawShader->AddUniform("source", INT);
}

void OpenGLSky::ProcessCube(GLSLShader* shader, GLuint cubemap, GLenum attachment)
{
    glm::mat4 V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    glm::mat3 IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubemap, 0); //negative Y
    OpenGLContent::getInstance()->DrawSAQ();
    
    V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>()/2.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemap, 0); //positive X
    OpenGLContent::getInstance()->DrawSAQ();
    
    V = glm::mat4();
    V = glm::rotate(V, -glm::pi<float>()/2.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubemap, 0); //negative Z
    OpenGLContent::getInstance()->DrawSAQ();
    
    V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubemap, 0);
    OpenGLContent::getInstance()->DrawSAQ();
    
    V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>()/2.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubemap, 0);
    OpenGLContent::getInstance()->DrawSAQ();
    
    V = glm::mat4();
    V = glm::rotate(V, -glm::pi<float>()/2.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubemap, 0);
    OpenGLContent::getInstance()->DrawSAQ();
}

void OpenGLSky::Generate(GLfloat elevation, GLfloat azimuth)
{
    sunElevation = elevation > 90.f ? 90.f : (elevation < -45.f ? -45.f : elevation);
    azimuth = (azimuth/360.f - truncf(azimuth/360.f)) * 360.f; //calculate in <-360, 360> range
    sunAzimuth = azimuth >= 0.f ? azimuth : 360.f + azimuth;   //remap to <0, 360> range
    
    OpenGLSun::getInstance()->SetPosition(sunElevation, sunAzimuth);
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    
    //Calculate projection
    glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 1.f, 100.f);
    projection = glm::inverse(projection);
    
    ///////Render Sky Cubemap
    glBindFramebuffer(GL_FRAMEBUFFER, skyCubeFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    //Setup viewport
    glViewport(0, 0, skyCubeSize, skyCubeSize);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //Light direction
    glm::mat4 lightMat;
    lightMat = glm::rotate(lightMat, glm::radians(90.f - sunAzimuth), glm::vec3(0.f,1.f,0.f)); //orientation
    lightMat = glm::rotate(lightMat, glm::radians(90.f - sunElevation), glm::vec3(1.f,0.f,0.f)); //elevation
    glm::vec4 lightDir = glm::vec4(0.f, 1.f, 0.f, 1.f);
    lightDir = lightMat * lightDir;
    
    skyCubeShader->Use();
    skyCubeShader->SetUniform("Kr", glm::vec3(0.18867780436772762f, 0.38f, 0.65f)); //skyCubeShader->SetUniform("Kr", glm::vec3(0.18867780436772762f, 0.4978442963618773f, 0.6616065586417131f));
    skyCubeShader->SetUniform("rayleigh_brightness", 33.f/10.f); //skyCubeShader->SetUniform("rayleigh_brightness", 33.f/10.f);
    skyCubeShader->SetUniform("rayleigh_strength", 139.f/1000.f);
    skyCubeShader->SetUniform("rayleigh_collection_power", 81.f/100.f);
    skyCubeShader->SetUniform("mie_brightness", 100.f/1000.f);
    skyCubeShader->SetUniform("mie_strength", 264.f/10000.f);
    skyCubeShader->SetUniform("mie_collection_power", 39.f/100.f);
    skyCubeShader->SetUniform("mie_distribution", 63.f/100.f);
    skyCubeShader->SetUniform("spot_brightness", 500.f/100.f); //skyCubeShader->SetUniform("spot_brightness", 1000.f/100.f);
    skyCubeShader->SetUniform("scatter_strength", 100.f/1000.f); //skyCubeShader->SetUniform("scatter_strength", 28.f/1000.f);
    skyCubeShader->SetUniform("lightdir", glm::vec3(lightDir));
    skyCubeShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize, (GLfloat)skyCubeSize));
    skyCubeShader->SetUniform("inv_proj", projection);
    ProcessCube(skyCubeShader, skyCubemap, GL_COLOR_ATTACHMENT0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    ////////Downsample Sky Cubemap x2
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    
    glBindFramebuffer(GL_FRAMEBUFFER, ds2FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    glViewport(0, 0, skyCubeSize/2, skyCubeSize/2);
    glClear(GL_COLOR_BUFFER_BIT);
    
    dsShader->Use();
    dsShader->SetUniform("source", 0);
    dsShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/2, (GLfloat)skyCubeSize/2));
    dsShader->SetUniform("inv_proj", projection);
    ProcessCube(dsShader, ds2Cubemap, GL_COLOR_ATTACHMENT0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    ///////////Downsample Sky Cubemap x4
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds2Cubemap);
    
    glBindFramebuffer(GL_FRAMEBUFFER, ds4FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    glViewport(0, 0, skyCubeSize/4, skyCubeSize/4);
    glClear(GL_COLOR_BUFFER_BIT);
   
    dsShader->Use();
    dsShader->SetUniform("source", 0);
    dsShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/4, (GLfloat)skyCubeSize/4));
    dsShader->SetUniform("inv_proj", projection);
    ProcessCube(dsShader, ds4Cubemap, GL_COLOR_ATTACHMENT0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ///////////Downsample Sky Cubemap x8
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds4Cubemap);
    
    glBindFramebuffer(GL_FRAMEBUFFER, ds8FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    dsShader->Use();
    dsShader->SetUniform("source", 0);
    dsShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/8, (GLfloat)skyCubeSize/8));
    dsShader->SetUniform("inv_proj", projection);
    ProcessCube(dsShader, ds8Cubemap, GL_COLOR_ATTACHMENT0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ///////////Convolve
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds8Cubemap);
    
    glBindFramebuffer(GL_FRAMEBUFFER, convolveFBO);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    convolveShader->Use();
    convolveShader->SetUniform("source", 0);
    convolveShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/8, (GLfloat)skyCubeSize/8));
    convolveShader->SetUniform("inv_proj", projection);
    convolveShader->SetUniform("specularity", 1.f);
    ProcessCube(convolveShader, convolveDiffuseCubemap, GL_COLOR_ATTACHMENT0);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    convolveShader->Use();
    convolveShader->SetUniform("source", 0);
    convolveShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/8, (GLfloat)skyCubeSize/8));
    convolveShader->SetUniform("inv_proj", projection);
    convolveShader->SetUniform("specularity", 100.f);
    ProcessCube(convolveShader, convolveReflectionCubemap, GL_COLOR_ATTACHMENT1);
    
	glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    ////////////////////////
}

void OpenGLSky::Render(OpenGLView *view, bool zAxisUp)
{
    GLint* viewport = view->GetViewport();
    glm::mat4 viewTransform = view->GetViewTransform();
	glm::mat4 projection = glm::perspective(view->GetFOVY(), (GLfloat)viewport[2]/(GLfloat)viewport[3], 0.1f, 100.f);
    glm::mat3 ivr = glm::mat3(glm::eulerAngleX(zAxisUp ? -M_PI_2 : M_PI_2)) * glm::inverse(glm::mat3(viewTransform)); 
	
    glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    
    skyDrawShader->Use();
    skyDrawShader->SetUniform("source", 0);
    skyDrawShader->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    skyDrawShader->SetUniform("inv_proj", glm::inverse(projection));
    skyDrawShader->SetUniform("inv_view_rot", ivr);
    OpenGLContent::getInstance()->DrawSAQ();
    glUseProgram(0);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
    
    delete viewport;
}

void OpenGLSky::ShowCubemap(SkyCubemap cmap)
{
    GLuint ctex = 0;
    switch (cmap)
    {
        case SKY:
        ctex = skyCubemap;
        break;
        
        case DOWNSAMPLE2:
        ctex = ds2Cubemap;
        break;
        
        case DOWNSAMPLE4:
        ctex = ds4Cubemap;
        break;
        
        case DOWNSAMPLE8:
        ctex = ds8Cubemap;
        break;
        
        case CONVOLUTION_DIFFUSE:
        ctex = convolveDiffuseCubemap;
        break;
            
        case CONVOLUTION_REFLECT:
        ctex = convolveReflectionCubemap;
        break;
    }
    
    OpenGLContent::getInstance()->DrawCubemapCross(ctex);
}

GLuint OpenGLSky::getDiffuseCubemap()
{
    return convolveDiffuseCubemap;
}

GLuint OpenGLSky::getReflectionCubemap()
{
    return convolveReflectionCubemap;
}

