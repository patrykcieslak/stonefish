//
//  OpenGLSky.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 9/18/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSky.h"
#include "GeometryUtil.h"
#include "OpenGLSolids.h"
#include "OpenGLSun.h"
#include "Console.h"

//Sky Cubemap
GLuint OpenGLSky::skyCubeFBO = 0;
GLuint OpenGLSky::skyCubemap = 0;
GLsizei OpenGLSky::skyCubeSize = 512;
GLSLShader* OpenGLSky::skyCubeShader = NULL;

//Downsampling
GLuint OpenGLSky::ds2FBO = 0;
GLuint OpenGLSky::ds2Cubemap = 0;
GLuint OpenGLSky::ds4FBO = 0;
GLuint OpenGLSky::ds4Cubemap = 0;
GLuint OpenGLSky::ds8FBO = 0;
GLuint OpenGLSky::ds8Cubemap = 0;
GLSLShader* OpenGLSky::dsShader = NULL;

//Convolution
GLuint OpenGLSky::convolveFBO = 0;
GLuint OpenGLSky::convolveDiffuseCubemap = 0;
GLuint OpenGLSky::convolveReflectionCubemap = 0;
GLSLShader* OpenGLSky::convolveShader = NULL;

//Sky drawing
GLSLShader* OpenGLSky::skyDrawShader = NULL;

//misc
GLfloat OpenGLSky::debugAngle = 0;

void OpenGLSky::Init()
{
    ////////Generate FBOs
    //Sky Cubemap
    glGenFramebuffersEXT(1, &skyCubeFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, skyCubeFBO);
    
    glGenTextures(1, &skyCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize, skyCubeSize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, skyCubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Sky FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    //Downsampling
    //x2
    glGenFramebuffersEXT(1, &ds2FBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds2FBO);
    
    glGenTextures(1, &ds2Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds2Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/2, skyCubeSize/2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ds2Cubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Sky downsample x2 FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    //x4
    glGenFramebuffersEXT(1, &ds4FBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds4FBO);
    
    glGenTextures(1, &ds4Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds4Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/4, skyCubeSize/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ds4Cubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Sky downsample x4 FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    //x8
    glGenFramebuffersEXT(1, &ds8FBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds8FBO);
    
    glGenTextures(1, &ds8Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds8Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/8, skyCubeSize/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ds8Cubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Sky downsample x8 FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    //Convolution
    glGenFramebuffersEXT(1, &convolveFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, convolveFBO);
    
    glGenTextures(1, &convolveDiffuseCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, convolveDiffuseCubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyCubeSize/8, skyCubeSize/8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, convolveDiffuseCubemap, 0);
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
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, convolveReflectionCubemap, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Sky convolve FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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

void OpenGLSky::Destroy()
{
    glDeleteTextures(1, &skyCubemap);
    glDeleteTextures(1, &ds2Cubemap);
    glDeleteTextures(1, &ds4Cubemap);
    glDeleteTextures(1, &ds8Cubemap);
    glDeleteTextures(1, &convolveDiffuseCubemap);
    glDeleteTextures(1, &convolveReflectionCubemap);
    glDeleteFramebuffersEXT(1, &skyCubeFBO);
    glDeleteFramebuffersEXT(1, &ds2FBO);
    glDeleteFramebuffersEXT(1, &ds4FBO);
    glDeleteFramebuffersEXT(1, &ds8FBO);
    glDeleteFramebuffersEXT(1, &convolveFBO);
    delete skyCubeShader;
    delete convolveShader;
    delete dsShader;
    delete skyDrawShader;
}

void OpenGLSky::ProcessCube(GLSLShader* shader, GLuint cubemap, GLenum attachment)
{
    glm::mat4 V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    glm::mat3 IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubemap, 0); //negative Y
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>()/2.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemap, 0); //positive X
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, -glm::pi<float>()/2.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubemap, 0); //negative Z
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, glm::pi<float>()/2.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, -glm::pi<float>()/2.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, glm::pi<float>(), glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = glm::inverse(IVR);
    shader->SetUniform("inv_view_rot", IVR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
}

void OpenGLSky::Generate(GLfloat elevation, GLfloat orientation)
{
    OpenGLSun::SetPosition(elevation, orientation);
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_SCISSOR_TEST);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    
    //Setup matrices
    OpenGLSolids::SetupOrtho();
   
    //Calculate projection
    glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 1.f, 100.f);
    projection = glm::inverse(projection);
    
    ///////Render Sky Cubemap
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, skyCubeFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    //Setup viewport
    glViewport(0, 0, skyCubeSize, skyCubeSize);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //Light direction
    glm::mat4 lightMat;
    lightMat = glm::rotate(lightMat, glm::radians(orientation), glm::vec3(0.f,1.f,0.f)); //orientation
    lightMat = glm::rotate(lightMat, glm::radians(90.f-elevation), glm::vec3(1.f,0.f,0.f)); //elevation
    glm::vec4 lightDir = glm::vec4(0.f, 1.f, 0.f, 1.f);
    lightDir = lightMat * lightDir;
    
    skyCubeShader->Enable();
    skyCubeShader->SetUniform("Kr", glm::vec3(0.18867780436772762f, 0.38f, 0.65f)); //skyCubeShader->SetUniform("Kr", glm::vec3(0.18867780436772762f, 0.4978442963618773f, 0.6616065586417131f));
    skyCubeShader->SetUniform("rayleigh_brightness", 50.f/10.f); //skyCubeShader->SetUniform("rayleigh_brightness", 33.f/10.f);
    skyCubeShader->SetUniform("rayleigh_strength", 139.f/1000.f);
    skyCubeShader->SetUniform("rayleigh_collection_power", 81.f/100.f);
    skyCubeShader->SetUniform("mie_brightness", 100.f/1000.f);
    skyCubeShader->SetUniform("mie_strength", 264.f/10000.f);
    skyCubeShader->SetUniform("mie_collection_power", 39.f/100.f);
    skyCubeShader->SetUniform("mie_distribution", 63.f/100.f);
    skyCubeShader->SetUniform("spot_brightness", 500.f/100.f); //skyCubeShader->SetUniform("spot_brightness", 1000.f/100.f);
    skyCubeShader->SetUniform("scatter_strength", 50.f/1000.f); //skyCubeShader->SetUniform("scatter_strength", 28.f/1000.f);
    skyCubeShader->SetUniform("lightdir", glm::vec3(lightDir));
    skyCubeShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize, (GLfloat)skyCubeSize));
    skyCubeShader->SetUniform("inv_proj", projection);
    ProcessCube(skyCubeShader, skyCubemap, GL_COLOR_ATTACHMENT0_EXT);
    skyCubeShader->Disable();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    ////////Downsample Sky Cubemap x2
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds2FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    glViewport(0, 0, skyCubeSize/2, skyCubeSize/2);
    glClear(GL_COLOR_BUFFER_BIT);
    
    dsShader->Enable();
    dsShader->SetUniform("source", 0);
    dsShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/2, (GLfloat)skyCubeSize/2));
    dsShader->SetUniform("inv_proj", projection);
    ProcessCube(dsShader, ds2Cubemap, GL_COLOR_ATTACHMENT0_EXT);
    dsShader->Disable();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    ///////////Downsample Sky Cubemap x4
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds2Cubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds4FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    glViewport(0, 0, skyCubeSize/4, skyCubeSize/4);
    glClear(GL_COLOR_BUFFER_BIT);
   
    dsShader->Enable();
    dsShader->SetUniform("source", 0);
    dsShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/4, (GLfloat)skyCubeSize/4));
    dsShader->SetUniform("inv_proj", projection);
    ProcessCube(dsShader, ds4Cubemap, GL_COLOR_ATTACHMENT0_EXT);
    dsShader->Disable();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ///////////Downsample Sky Cubemap x8
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds4Cubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds8FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    dsShader->Enable();
    dsShader->SetUniform("source", 0);
    dsShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/8, (GLfloat)skyCubeSize/8));
    dsShader->SetUniform("inv_proj", projection);
    ProcessCube(dsShader, ds8Cubemap, GL_COLOR_ATTACHMENT0_EXT);
    dsShader->Disable();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ///////////Convolve
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds8Cubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, convolveFBO);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    convolveShader->Enable();
    convolveShader->SetUniform("source", 0);
    convolveShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/8, (GLfloat)skyCubeSize/8));
    convolveShader->SetUniform("inv_proj", projection);
    convolveShader->SetUniform("specularity", 1.f);
    ProcessCube(convolveShader, convolveDiffuseCubemap, GL_COLOR_ATTACHMENT0_EXT);
    convolveShader->Disable();
    
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    convolveShader->Enable();
    convolveShader->SetUniform("source", 0);
    convolveShader->SetUniform("viewport", glm::vec2((GLfloat)skyCubeSize/8, (GLfloat)skyCubeSize/8));
    convolveShader->SetUniform("inv_proj", projection);
    convolveShader->SetUniform("specularity", 100.f);
    ProcessCube(convolveShader, convolveReflectionCubemap, GL_COLOR_ATTACHMENT1_EXT);
    convolveShader->Disable();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    ////////////////////////
    
    glPopAttrib();
}

void OpenGLSky::Render(OpenGLView *view, const btTransform& viewTransform, bool zAxisUp)
{
    GLint* viewport = view->GetViewport();
    
    glm::mat4 projection = glm::perspective(view->GetFOVY(), (GLfloat)viewport[2]/(GLfloat)viewport[3], 0.1f, 100.f);
    projection = glm::inverse(projection);
    
    btMatrix3x3 flip;
    flip.setEulerZYX(zAxisUp ? -M_PI_2 : M_PI_2, 0, 0);
    flip = flip * viewTransform.getBasis().inverse();
    
    GLfloat IVRMatrix[9];
    SetFloatvFromMat(flip, IVRMatrix);
    glm::mat3 ivr = glm::make_mat3(IVRMatrix);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    
    skyDrawShader->Enable();
    skyDrawShader->SetUniform("source", 0);
    skyDrawShader->SetUniform("viewport", glm::vec2((GLfloat)(viewport[2]-viewport[0]), (GLfloat)(viewport[3]-viewport[1])));
    skyDrawShader->SetUniform("inv_proj", projection);
    skyDrawShader->SetUniform("inv_view_rot", ivr);
    OpenGLSolids::DrawScreenAlignedQuad();
    skyDrawShader->Disable();
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
    
    delete viewport;
}

void OpenGLSky::ShowCubemap(SkyCubemap cmap, GLfloat x, GLfloat y, GLfloat width, GLfloat height)
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
    
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glViewport(x, y, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    //Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.f, 10.f, 1000000000.f);
    glLoadMatrixf(glm::value_ptr(proj));
	
	//Model setup
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ctex);
    
	// Render the quad
	glLoadIdentity();
	glTranslatef(0,0,-35.f);
    glRotatef(2.f*debugAngle, 1.0, 0, 0);
    glRotatef(debugAngle, 0, 1.0, 0);
	debugAngle += 1.f;
    
    glColor3f(1.f,1.f,1.f);
    GLfloat d=-10;

    glBegin(GL_QUADS);
    //xNeg
    glTexCoord3f(-1.f,  1.f, -1.f); glVertex3f(-d,  d, -d);
    glTexCoord3f(-1.f,  1.f,  1.f); glVertex3f(-d,  d,  d);
    glTexCoord3f(-1.f, -1.f,  1.f); glVertex3f(-d, -d,  d);
    glTexCoord3f(-1.f, -1.f, -1.f); glVertex3f(-d, -d, -d);
    
    //xPos
    glTexCoord3f(1.f,  1.f,  1.f); glVertex3f(d,  d,  d);
    glTexCoord3f(1.f,  1.f, -1.f); glVertex3f(d,  d, -d);
    glTexCoord3f(1.f, -1.f, -1.f); glVertex3f(d, -d, -d);
    glTexCoord3f(1.f, -1.f,  1.f); glVertex3f(d, -d,  d);
    
    //zNeg
    glTexCoord3f( 1.f,  1.f, -1.f); glVertex3f( d,  d, -d);
    glTexCoord3f(-1.f,  1.f, -1.f); glVertex3f(-d,  d, -d);
    glTexCoord3f(-1.f, -1.f, -1.f); glVertex3f(-d, -d, -d);
    glTexCoord3f( 1.f, -1.f, -1.f); glVertex3f( d, -d, -d);
    
    //zPos
    glTexCoord3f(-1.f,  1.f, 1.f); glVertex3f(-d,  d,  d);
    glTexCoord3f( 1.f,  1.f, 1.f); glVertex3f( d,  d,  d);
    glTexCoord3f( 1.f, -1.f, 1.f); glVertex3f( d, -d,  d);
    glTexCoord3f(-1.f, -1.f, 1.f); glVertex3f(-d, -d,  d);
    
    //yNeg
    glTexCoord3f(-1.f, -1.f, -1.f); glVertex3f(-d, -d, -d);
    glTexCoord3f(-1.f, -1.f,  1.f); glVertex3f(-d, -d,  d);
    glTexCoord3f( 1.f, -1.f,  1.f); glVertex3f( d, -d,  d);
    glTexCoord3f( 1.f, -1.f, -1.f); glVertex3f( d, -d, -d);
    
    //yPos
    glTexCoord3f( 1.f, 1.f, -1.f); glVertex3f( d,  d, -d);
    glTexCoord3f( 1.f, 1.f,  1.f); glVertex3f( d,  d,  d);
    glTexCoord3f(-1.f, 1.f,  1.f); glVertex3f(-d,  d,  d);
    glTexCoord3f(-1.f, 1.f, -1.f); glVertex3f(-d,  d, -d);
    glEnd();
    
    
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
    
	//Reset to the matrices
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

GLuint OpenGLSky::getDiffuseCubemap()
{
    return convolveDiffuseCubemap;
}

GLuint OpenGLSky::getReflectionCubemap()
{
    return convolveReflectionCubemap;
}

//private constructor - unused
OpenGLSky::OpenGLSky()
{
}


