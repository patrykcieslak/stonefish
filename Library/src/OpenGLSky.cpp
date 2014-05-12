//
//  OpenGLSky.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 9/18/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLSky.h"
#include "OpenGLUtil.h"
#include "OpenGLSolids.h"
#include "OpenGLSun.h"

//Sky Cubemap
GLuint OpenGLSky::skyCubeFBO = 0;
GLuint OpenGLSky::skyCubemap = 0;
GLsizei OpenGLSky::skyCubeSize = 512;
GLhandleARB OpenGLSky::skyCubeShader = NULL;
GLint OpenGLSky::uniSCViewport = 0;
GLint OpenGLSky::uniSCIP = 0;
GLint OpenGLSky::uniSCIVR = 0;
GLint OpenGLSky::uniLightDir = 0;
GLint OpenGLSky::uniKr = 0;
GLint OpenGLSky::uniRayleighBrightness = 0;
GLint OpenGLSky::uniMieBrightness = 0;
GLint OpenGLSky::uniSpotBrightness = 0;
GLint OpenGLSky::uniScatterStrength = 0;
GLint OpenGLSky::uniRayleighStrength = 0;
GLint OpenGLSky::uniMieStrength = 0;
GLint OpenGLSky::uniRayleighCollection = 0;
GLint OpenGLSky::uniMieCollection = 0;
GLint OpenGLSky::uniMieDistribution = 0;

//Downsampling
GLuint OpenGLSky::ds2FBO = 0;
GLuint OpenGLSky::ds2Cubemap = 0;
GLuint OpenGLSky::ds4FBO = 0;
GLuint OpenGLSky::ds4Cubemap = 0;
GLuint OpenGLSky::ds8FBO = 0;
GLuint OpenGLSky::ds8Cubemap = 0;
GLhandleARB OpenGLSky::dsShader = NULL;
GLint OpenGLSky::uniDsViewport = 0;
GLint OpenGLSky::uniDsIP = 0;
GLint OpenGLSky::uniDsIVR = 0;
GLint OpenGLSky::uniDsSampler = 0;

//Convolution
GLuint OpenGLSky::convolveFBO = 0;
GLuint OpenGLSky::convolveDiffuseCubemap = 0;
GLuint OpenGLSky::convolveReflectionCubemap = 0;
GLhandleARB OpenGLSky::convolveShader = NULL;
GLint OpenGLSky::uniConvolveViewport = 0;
GLint OpenGLSky::uniConvolveIP = 0;
GLint OpenGLSky::uniConvolveIVR = 0;
GLint OpenGLSky::uniConvolveSampler = 0;
GLint OpenGLSky::uniConvolveSpecularity = 0;

//Sky drawing
GLhandleARB OpenGLSky::skyDrawShader = NULL;
GLint OpenGLSky::uniSkyDrawViewport = 0;
GLint OpenGLSky::uniSkyDrawIP = 0;
GLint OpenGLSky::uniSkyDrawIVR = 0;
GLint OpenGLSky::uniSkyDrawSampler = 0;

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
        printf("Sky FBO initialization failed.\n");
    
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
        printf("Downsample x2 FBO initialization failed.\n");
    
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
        printf("Downsample x4 FBO initialization failed.\n");
    
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
        printf("Downsample x8 FBO initialization failed.\n");
    
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
        printf("Convolve FBO initialization failed.\n");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    ////////Load shaders
    //Sky Cubemap
    GLhandleARB vertexShader, fragmentShader;
    GLint compiled;
    vertexShader = LoadShader(GL_VERTEX_SHADER, "saq.vert", &compiled);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "cubeSky.frag", &compiled);
    skyCubeShader = CreateProgramObject(vertexShader, fragmentShader);
    LinkProgram(skyCubeShader, &compiled);
    
    glUseProgramObjectARB(skyCubeShader);
    uniSCViewport  = glGetUniformLocationARB(skyCubeShader, "viewport");
    uniSCIP  = glGetUniformLocationARB(skyCubeShader, "inv_proj");
    uniSCIVR  = glGetUniformLocationARB(skyCubeShader, "inv_view_rot");
    uniLightDir  = glGetUniformLocationARB(skyCubeShader, "lightdir");
    uniKr  = glGetUniformLocationARB(skyCubeShader, "Kr");
    uniRayleighBrightness  = glGetUniformLocationARB(skyCubeShader, "rayleigh_brightness");
    uniMieBrightness  = glGetUniformLocationARB(skyCubeShader, "mie_brightness");
    uniSpotBrightness  = glGetUniformLocationARB(skyCubeShader, "spot_brightness");
    uniScatterStrength  = glGetUniformLocationARB(skyCubeShader, "scatter_strength");
    uniRayleighStrength  = glGetUniformLocationARB(skyCubeShader, "rayleigh_strength");
    uniMieStrength  = glGetUniformLocationARB(skyCubeShader, "mie_strength");
    uniRayleighCollection  = glGetUniformLocationARB(skyCubeShader, "rayleigh_collection_power");
    uniMieCollection = glGetUniformLocationARB(skyCubeShader, "mie_collection_power");
    uniMieDistribution  = glGetUniformLocationARB(skyCubeShader, "mie_distribution");
    glUseProgramObjectARB(0);
    
    //Downsample
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "cubeDownsample.frag", &compiled);
    dsShader = CreateProgramObject(vertexShader, fragmentShader);
    LinkProgram(dsShader, &compiled);
    
    glUseProgramObjectARB(dsShader);
    uniDsViewport  = glGetUniformLocationARB(dsShader, "viewport");
    uniDsIP  = glGetUniformLocationARB(dsShader, "inv_proj");
    uniDsIVR  = glGetUniformLocationARB(dsShader, "inv_view_rot");
    uniDsSampler  = glGetUniformLocationARB(dsShader, "source");
    glUseProgramObjectARB(0);
    
    //Convolve
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "cubeConvolve.frag", &compiled);
    convolveShader = CreateProgramObject(vertexShader, fragmentShader);
    LinkProgram(convolveShader, &compiled);
    
    glUseProgramObjectARB(convolveShader);
    uniConvolveViewport  = glGetUniformLocationARB(convolveShader, "viewport");
    uniConvolveIP  = glGetUniformLocationARB(convolveShader, "inv_proj");
    uniConvolveIVR  = glGetUniformLocationARB(convolveShader, "inv_view_rot");
    uniConvolveSampler  = glGetUniformLocationARB(convolveShader, "source");
    uniConvolveSpecularity = glGetUniformLocationARB(convolveShader, "specularity");
    glUseProgramObjectARB(0);
    
    //Draw Sky
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "cubePass.frag", &compiled);
    skyDrawShader = CreateProgramObject(vertexShader, fragmentShader);
    LinkProgram(skyDrawShader, &compiled);
    
    glUseProgramObjectARB(skyDrawShader);
    uniSkyDrawViewport  = glGetUniformLocationARB(skyDrawShader, "viewport");
    uniSkyDrawIP  = glGetUniformLocationARB(skyDrawShader, "inv_proj");
    uniSkyDrawIVR  = glGetUniformLocationARB(skyDrawShader, "inv_view_rot");
    uniSkyDrawSampler  = glGetUniformLocationARB(skyDrawShader, "source");
    glUseProgramObjectARB(0);
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
    
    if(skyCubeShader != NULL)
        glDeleteObjectARB(skyCubeShader);
    
    if(dsShader != NULL)
        glDeleteObjectARB(dsShader);
    
    if(convolveShader != NULL)
        glDeleteObjectARB(convolveShader);
}

void OpenGLSky::ProcessCube(GLint ivrUniform, GLuint cubemap, GLenum attachment)
{
    glm::mat4 V = glm::mat4();
    V = glm::rotate(V, 180.f, glm::vec3(1.f,0.f,0.f));
    glm::mat3 IVR = glm::mat3(V);
    IVR = IVR._inverse();
    glUniformMatrix3fv(ivrUniform, 1, GL_FALSE, glm::value_ptr(IVR));
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, 90.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, 180.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = IVR._inverse();
    glUniformMatrix3fv(ivrUniform, 1, GL_FALSE, glm::value_ptr(IVR));
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, 90.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = IVR._inverse();
    glUniformMatrix3fv(ivrUniform, 1, GL_FALSE, glm::value_ptr(IVR));
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, 180.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, 180.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = IVR._inverse();
    glUniformMatrix3fv(ivrUniform, 1, GL_FALSE, glm::value_ptr(IVR));
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, -90.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = IVR._inverse();
    glUniformMatrix3fv(ivrUniform, 1, GL_FALSE, glm::value_ptr(IVR));
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubemap, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    V = glm::mat4();
    V = glm::rotate(V, -90.f, glm::vec3(0.f,1.f,0.f));
    V = glm::rotate(V, 180.f, glm::vec3(1.f,0.f,0.f));
    IVR = glm::mat3(V);
    IVR = IVR._inverse();
    glUniformMatrix3fv(ivrUniform, 1, GL_FALSE, glm::value_ptr(IVR));
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Calculate projection
    glm::mat4 projection = glm::perspective(90.f, 1.f, 1.f, 100.f);
    projection = glm::inverse(projection);
    
    ///////Render Sky Cubemap
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, skyCubeFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    //Setup viewport
    glViewport(0, 0, skyCubeSize, skyCubeSize);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //Light direction
    glm::mat4 lightMat;
    lightMat = glm::rotate(lightMat, 90.f-elevation, glm::vec3(1.f,0.f,0.f)); //elevation
    lightMat = glm::rotate(lightMat, orientation, glm::vec3(0.f,1.f,0.f)); //orientation
    glm::vec4 lightDir = glm::vec4(0.f,1.f,0.f,0.f);
    lightDir = glm::mul(lightDir, lightMat);
    
    glUseProgramObjectARB(skyCubeShader);
    //glUniform3f(uniKr, 0.18867780436772762f, 0.4978442963618773f, 0.6616065586417131f);
    glUniform3f(uniKr, 0.18, 0.38, 0.65f);
    //glUniform1f(uniRayleighBrightness, 33.f/10.f);
    glUniform1f(uniRayleighBrightness, 50.f/10.f);
    glUniform1f(uniRayleighStrength, 139.f/1000.f);
    glUniform1f(uniRayleighCollection, 81.f/100.f);
    //glUniform1f(uniMieBrightness, 100.f/1000.f);
    glUniform1f(uniMieBrightness, 100.f/1000.f);
    glUniform1f(uniMieStrength, 264.f/10000.f);
    glUniform1f(uniMieCollection, 39.f/100.f);
    glUniform1f(uniMieDistribution, 63.f/100.f);
    //glUniform1f(uniSpotBrightness, 1000.f/100.f);
    glUniform1f(uniSpotBrightness, 500.f/100.f);
    //glUniform1f(uniScatterStrength, 28.f/1000.f);
    glUniform1f(uniScatterStrength, 50.f/1000.f);
    
    glUniform3fv(uniLightDir, 1, glm::value_ptr(glm::vec3(lightDir)));
    glUniform2f(uniSCViewport, skyCubeSize, skyCubeSize);
    glUniformMatrix4fv(uniSCIP, 1, GL_FALSE, glm::value_ptr(projection));
    ProcessCube(uniSCIVR, skyCubemap, GL_COLOR_ATTACHMENT0_EXT);
    glUseProgramObjectARB(0);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    ////////Downsample Sky Cubemap x2
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds2FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    glViewport(0, 0, skyCubeSize/2, skyCubeSize/2);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgramObjectARB(dsShader);
    glUniform1i(uniDsSampler, 0);
    glUniform2f(uniDsViewport, skyCubeSize/2, skyCubeSize/2);
    glUniformMatrix4fv(uniDsIP, 1, GL_FALSE, glm::value_ptr(projection));
    ProcessCube(uniDsIVR, ds2Cubemap, GL_COLOR_ATTACHMENT0_EXT);
    glUseProgramObjectARB(0);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    ///////////Downsample Sky Cubemap x4
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds2Cubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds4FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    glViewport(0, 0, skyCubeSize/4, skyCubeSize/4);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgramObjectARB(dsShader);
    glUniform1i(uniDsSampler, 0);
    glUniform2f(uniDsViewport, skyCubeSize/4, skyCubeSize/4);
    glUniformMatrix4fv(uniDsIP, 1, GL_FALSE, glm::value_ptr(projection));
    ProcessCube(uniDsIVR, ds4Cubemap, GL_COLOR_ATTACHMENT0_EXT);
    glUseProgramObjectARB(0);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ///////////Downsample Sky Cubemap x8
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds4Cubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ds8FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgramObjectARB(dsShader);
    glUniform1i(uniDsSampler, 0);
    glUniform2f(uniDsViewport, skyCubeSize/8, skyCubeSize/8);
    glUniformMatrix4fv(uniDsIP, 1, GL_FALSE, glm::value_ptr(projection));
    ProcessCube(uniDsIVR, ds8Cubemap, GL_COLOR_ATTACHMENT0_EXT);
    glUseProgramObjectARB(0);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ///////////Convolve
    glBindTexture(GL_TEXTURE_CUBE_MAP, ds8Cubemap);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, convolveFBO);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgramObjectARB(convolveShader);
    glUniform1i(uniConvolveSampler, 0);
    glUniform1f(uniConvolveSpecularity, 1.f);
    glUniform2f(uniConvolveViewport, skyCubeSize/8.f, skyCubeSize/8.f);
    glUniformMatrix4fv(uniConvolveIP, 1, GL_FALSE, glm::value_ptr(projection));
    ProcessCube(uniConvolveIVR, convolveDiffuseCubemap, GL_COLOR_ATTACHMENT0_EXT);
    glUseProgramObjectARB(0);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glViewport(0, 0, skyCubeSize/8, skyCubeSize/8);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgramObjectARB(convolveShader);
    glUniform1i(uniConvolveSampler, 0);
    glUniform1f(uniConvolveSpecularity, 100.f);
    glUniform2f(uniConvolveViewport, skyCubeSize/8.f, skyCubeSize/8.f);
    glUniformMatrix4fv(uniConvolveIP, 1, GL_FALSE, glm::value_ptr(projection));
    ProcessCube(uniConvolveIVR, convolveReflectionCubemap, GL_COLOR_ATTACHMENT1_EXT);
    glUseProgramObjectARB(0);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    ////////////////////////
    
    glPopAttrib();
    
    printf("Sky generated.\n");
}

void OpenGLSky::Render(OpenGLView *view, const btTransform& viewTransform, bool zAxisUp)
{
    GLint* viewport = view->GetViewport();
    GLfloat fovy = view->GetFOVY()/M_PI*180.f;
    
    glm::mat4 projection = glm::perspective(fovy, (GLfloat)viewport[2]/(GLfloat)viewport[3], 1.f, 100.f);
    projection = glm::inverse(projection);
    
    btMatrix3x3 flip;
    flip.setEulerZYX(zAxisUp ? -M_PI_2 : M_PI_2, 0, 0);
    flip = flip * viewTransform.getBasis().inverse();
    
    GLfloat IVRMatrix[9];
    SetFloatvFromMat(flip, IVRMatrix);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
    
    glUseProgramObjectARB(skyDrawShader);
    glUniform2f(uniSkyDrawViewport,viewport[2]-viewport[0], viewport[3]-viewport[1]);
    glUniformMatrix4fv(uniSkyDrawIP, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix3fv(uniSkyDrawIVR, 1, GL_FALSE, IVRMatrix);
    glUniform1i(uniSkyDrawSampler, 0);
    OpenGLSolids::DrawScreenAlignedQuad();
    glUseProgramObjectARB(0);

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
	glLoadIdentity();
	gluPerspective(90, 1, 10.0, 100000000.0);
    
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


