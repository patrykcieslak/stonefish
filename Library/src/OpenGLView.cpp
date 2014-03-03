//
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLView.h"
#include "OpenGLSolids.h"
#include "OpenGLUtil.h"
#include "GeometryUtil.h"

GLhandleARB OpenGLView::ssaoShader = NULL;
GLhandleARB OpenGLView::blurShader = NULL;
GLhandleARB OpenGLView::fluidShader[] = {NULL, NULL, NULL, NULL};
GLuint OpenGLView::randomTexture = 0;
GLint OpenGLView::randomTextureUnit = -1;
GLint OpenGLView::normalTextureUnit = -1;
GLint OpenGLView::uniSsaoNormal = -1;
GLint OpenGLView::uniSsaoRandom = -1;
GLint OpenGLView::uniSsaoEpsilon = -1;
GLint OpenGLView::uniSsaoViewport = -1;
GLint OpenGLView::uniSsaoIP = -1;
GLint OpenGLView::uniSsaoIVR = -1;
GLint OpenGLView::uniSsaoP = -1;
GLint OpenGLView::uniSsaoRandomSize = -1;
GLint OpenGLView::uniSsaoFullOcclTh = -1;
GLint OpenGLView::uniSsaoNoOcclTh = -1;
GLint OpenGLView::uniSsaoOcclPower = -1;
GLint OpenGLView::uniSsaoRadius = -1;
GLint OpenGLView::uniBlurViewport = -1;
GLint OpenGLView::uniBlurSource = -1;
GLint OpenGLView::uniBlurNormal = -1;
GLint OpenGLView::uniBlurAxis = -1;
GLint OpenGLView::uniBlurNormalFactor = -1;
GLint OpenGLView::uniBlurNormalPower = -1;
GLint OpenGLView::uniBlurDepthPower = -1;
GLint OpenGLView::uniBlurDistanceFactor = -1;
GLint OpenGLView::uniFluidReflection[] = {-1,-1};
GLint OpenGLView::uniFluidPosition[] = {-1,-1,-1};
GLint OpenGLView::uniFluidViewport[] = {-1,-1,-1};
GLint OpenGLView::uniFluidIP[] = {-1,-1};
GLint OpenGLView::uniFluidR0[] = {-1,-1};
GLint OpenGLView::uniFluidScene[] = {-1,-1,-1,-1};
GLint OpenGLView::uniFluidEyeSurfaceN[] = {-1,-1,-1};
GLint OpenGLView::uniFluidEyeSurfaceP[] = {-1,-1,-1};
GLint OpenGLView::uniFluidVisibility[] = {-1,-1,-1};

OpenGLView::OpenGLView(GLint x, GLint y, GLint width, GLint height, GLuint ssaoSize)
{
    viewportWidth = width;
    viewportHeight = height;
    originX = x;
    originY = y;
    fovx = 0.785f;
    active = false;
    ssaoSizeDiv = ssaoSize;
    gBuffer = new OpenGLGBuffer(viewportWidth, viewportHeight);
    far = 1000000.f;
    near = 1.f;
    
    glGenFramebuffersEXT(1, &sceneFBO);
    glGenRenderbuffersEXT(1, &sceneDepthBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, sceneFBO);
    
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, sceneDepthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, sceneDepthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
    glGenTextures(1, &sceneTexture);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, sceneTexture, 0);
    
    glGenTextures(1, &sceneReflectionTexture);
    glBindTexture(GL_TEXTURE_2D, sceneReflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, sceneReflectionTexture, 0);
    
    glGenTextures(1, &sceneRefractionTexture);
    glBindTexture(GL_TEXTURE_2D, sceneRefractionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, sceneRefractionTexture, 0);
    
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        printf("Scene FBO initialization failed.\n");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    if(ssaoSizeDiv > 0)
    {
        glGenFramebuffersEXT(1, &ssaoFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ssaoFBO);
    
        glGenTextures(1, &ssaoTexture);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ssaoTexture, 0);
    
        GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
            printf("SSAO FBO initialization failed.\n");
    
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
        glGenFramebuffersEXT(1, &hBlurFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hBlurFBO);
    
        glGenTextures(1, &hBlurTexture);
        glBindTexture(GL_TEXTURE_2D, hBlurTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, hBlurTexture, 0);
    
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
            printf("Horizontal blur FBO initialization failed.\n");
    
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
        glGenFramebuffersEXT(1, &vBlurFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, vBlurFBO);
    
        glGenTextures(1, &vBlurTexture);
        glBindTexture(GL_TEXTURE_2D, vBlurTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, vBlurTexture, 0);
    
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
            printf("Vertical blur FBO initialization failed.\n");
    
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
}

OpenGLView::~OpenGLView()
{
    glDeleteRenderbuffersEXT(1, &sceneDepthBuffer);
    glDeleteTextures(1, &sceneTexture);
    glDeleteTextures(1, &sceneReflectionTexture);
    glDeleteTextures(1, &sceneRefractionTexture);
    glDeleteFramebuffersEXT(1, &sceneFBO);
    
    if(ssaoSizeDiv > 0)
    {
        glDeleteTextures(1, &ssaoTexture);
        glDeleteTextures(1, &hBlurTexture);
        glDeleteTextures(1, &vBlurTexture);
        glDeleteFramebuffersEXT(1, &sceneFBO);
        glDeleteFramebuffersEXT(1, &ssaoFBO);
        glDeleteFramebuffersEXT(1, &hBlurFBO);
        glDeleteFramebuffersEXT(1, &vBlurFBO);
    }
    
    delete gBuffer;
}

void OpenGLView::Activate()
{
    active = true;
}

void OpenGLView::Deactivate()
{
    active = false;
}

bool OpenGLView::isActive()
{
    return active;
}

GLint* OpenGLView::GetViewport()
{
	GLint* view = new GLint[4];
	view[0] = originX;
	view[1] = originY;
	view[2] = viewportWidth;
	view[3] = viewportHeight;
    return view;
}

glm::mat4 OpenGLView::GetProjection()
{
    return projection;
}

GLfloat OpenGLView::GetFOVY()
{
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    return fovx/aspect;
}

GLfloat OpenGLView::GetNearClip()
{
    return near;
}

GLfloat OpenGLView::GetFarClip()
{
    return far;
}

OpenGLGBuffer* OpenGLView::getGBuffer()
{
    return gBuffer;
}

GLuint OpenGLView::getSceneFBO()
{
    return sceneFBO;
}

GLuint OpenGLView::getSceneTexture()
{
    return sceneTexture;
}

GLuint OpenGLView::getSceneReflectionTexture()
{
    return sceneReflectionTexture;
}

GLuint OpenGLView::getSceneRefractionTexture()
{
    return sceneRefractionTexture;
}

void OpenGLView::SetupViewport(GLint x, GLint y, GLint width)
{
    originX = x;
    originY = y;
    GLint oldWidth = viewportWidth;
    viewportWidth = width;
    viewportHeight = ((GLfloat)viewportHeight/(GLfloat)oldWidth)*width;
    
    delete gBuffer;
    gBuffer = new OpenGLGBuffer(viewportWidth, viewportHeight);
}

void OpenGLView::SetViewport()
{
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void OpenGLView::SetProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));
}

void OpenGLView::SetViewTransform()
{
    btTransform trans = GetViewTransform();
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
#ifdef BT_USE_DOUBLE_PRECISION
    glLoadMatrixd(openglTrans);
#else
    glLoadMatrixf(openglTrans);
#endif
    
    gBuffer->SetClipPlane(NULL);
}

void OpenGLView::SetReflectedViewTransform(FluidEntity* fluid)
{
    btVector3 surfN, surfP;
    fluid->GetSurface(surfN, surfP);
    
    btScalar eyeDepth = distanceFromCenteredPlane(surfN, surfP-GetEyePosition());
    if(eyeDepth >= 0)
        surfN = -surfN;
    
    btTransform trans = GetReflectedViewTransform(surfN, surfP);
    
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
#ifdef BT_USE_DOUBLE_PRECISION
    glLoadMatrixd(openglTrans);
#else
    glLoadMatrixf(openglTrans);
#endif
    
    glFrontFace(GL_CW);
    
    //calculate eye space clip plane equation
    GLfloat planeEq[4];
    btVector3 transPlaneP = trans * surfP;
    btVector3 transPlaneN = trans.getBasis().inverse().transpose() * surfN;
    planeEq[0] = transPlaneN.x();
    planeEq[1] = transPlaneN.y();
    planeEq[2] = transPlaneN.z();
    planeEq[3] = -transPlaneN.dot(transPlaneP);
    gBuffer->SetClipPlane(planeEq);
}

void OpenGLView::SetRefractedViewTransform(FluidEntity* fluid)
{
    btVector3 surfN, surfP;
    fluid->GetSurface(surfN, surfP);

    btTransform trans = GetViewTransform();  //GetRefractedViewTransform(surfN, surfP, fluid->getFluid());
    btTransform shift = btTransform(btQuaternion(0,0,0), btVector3(0,0,0));
    
    btScalar openglTrans[16];
    (trans * shift).getOpenGLMatrix(openglTrans);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
#ifdef BT_USE_DOUBLE_PRECISION
    glLoadMatrixd(openglTrans);
#else
    glLoadMatrixf(openglTrans);
#endif
    
    //calculate eye space clip plane equation
    GLfloat planeEq[4];
    btVector3 transPlaneP = trans * surfP;
    btVector3 transPlaneN = trans.getBasis().inverse().transpose() * surfN;
    planeEq[0] = -transPlaneN.x();
    planeEq[1] = -transPlaneN.y();
    planeEq[2] = -transPlaneN.z();
    planeEq[3] = transPlaneN.dot(transPlaneP);
    gBuffer->SetClipPlane(planeEq);
}


btTransform OpenGLView::GetReflectedViewTransform(const btVector3 &normal, const btVector3 &position)
{
    //get camera transform
    btTransform trans = GetViewTransform();
    
    GLfloat planeEq[4];
    planeEq[0] = normal.x();
    planeEq[1] = normal.y();
    planeEq[2] = normal.z();
    planeEq[3] = -normal.dot(position);
    
    //create reflection transform
    btTransform reflection = btTransform::getIdentity();
    btMatrix3x3 rotation = btMatrix3x3(1.0-2.0*planeEq[0]*planeEq[0], -2.0*planeEq[0]*planeEq[1], -2.0*planeEq[0]*planeEq[2],
                                       -2.0*planeEq[0]*planeEq[1], 1.0-2.0*planeEq[1]*planeEq[1], -2.0*planeEq[1]*planeEq[2],
                                       -2.0*planeEq[0]*planeEq[2], -2.0*planeEq[1]*planeEq[2], 1.0-2.0*planeEq[2]*planeEq[2]);
    
    reflection.setBasis(rotation);
    reflection.setOrigin(-2.0*planeEq[3]*btVector3(planeEq[0], planeEq[1], planeEq[2]));
    
    //create flipping transform
    //btTransform flip = btTransform::getIdentity();
    //rotation = btMatrix3x3(1.0, 0.0, 0.0,
    //                       0.0, -1.0, 0.0,
    //                       0.0, 0.0, 1.0);
    //flip.setBasis(rotation);
    
    //combine transforms
    trans = trans * reflection;//flip * trans * reflection;
    return trans;
}

btTransform OpenGLView::GetRefractedViewTransform(const btVector3 &normal, const btVector3 &position, const Fluid* fluid)
{
    GLfloat planeEq[4];
    planeEq[0] = normal.x();
    planeEq[1] = normal.y();
    planeEq[2] = normal.z();
    planeEq[3] = -normal.dot(position);
    
    //get camera transform
    btVector3 dir = GetLookingDirection().normalize();
    btVector3 axis = dir.cross(normal);
    btScalar alpha = acos(normal.dot(-dir));
    btScalar beta = asin(axis.length()/fluid->IOR);
    axis = axis.normalize();
    
    btQuaternion rotation = btQuaternion(axis, alpha-beta);
    btTransform trans = GetViewTransform();
    
    //create reflection transform
    btTransform refraction = btTransform::getIdentity();
    refraction.setBasis(btMatrix3x3(rotation));

    
    //combine transforms
    trans = trans * refraction;
    return trans;
}

void OpenGLView::RenderSSAO()
{
    if(ssaoSizeDiv > 0)
    {
        glm::mat4 invProj= glm::inverse(projection);
        
        btTransform viewtrans = GetViewTransform();
        btMatrix3x3 ivr = viewtrans.getBasis().inverse();
        GLfloat IVRMatrix[9];
        SetFloatvFromMat(ivr, IVRMatrix);
        
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ssaoFBO);
        glViewport(0, 0, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        SetupOrtho();
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgramObjectARB(ssaoShader);
        glUniform1f(uniSsaoRadius, 100.f);
        glUniform1f(uniSsaoEpsilon, 0.01f);
        glUniform1f(uniSsaoFullOcclTh, 2.0f);
        glUniform1f(uniSsaoNoOcclTh, 50.f);
        glUniform1f(uniSsaoOcclPower, 2.0f);
        glUniform1f(uniSsaoRandomSize, 64.f);
        glUniform2f(uniSsaoViewport, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        glUniform1i(uniSsaoNormal, normalTextureUnit);
        glUniform1i(uniSsaoRandom, randomTextureUnit);
        glUniformMatrix4fv(uniSsaoP, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniSsaoIP, 1, GL_FALSE, glm::value_ptr(invProj));
        glUniformMatrix3fv(uniSsaoIVR, 1, GL_FALSE, IVRMatrix);
        DrawScreenAlignedQuad();
        glUseProgramObjectARB(0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        
        glActiveTextureARB(GL_TEXTURE0_ARB + randomTextureUnit);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        
        glUseProgramObjectARB(blurShader);
        glUniform2f(uniBlurViewport, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        glUniform1i(uniBlurSource, randomTextureUnit);
        glUniform1i(uniBlurNormal, normalTextureUnit);
        glUniform1f(uniBlurNormalPower, 1.f);
        glUniform1f(uniBlurNormalFactor, 1.f);
        glUniform1f(uniBlurDepthPower, 0.5f);
        glUniform1f(uniBlurDistanceFactor, 100.f);
        glUniform2f(uniBlurAxis, 1.2f, 0.f);
        
        for(int i=0; i<4; i++)
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hBlurFBO);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClear(GL_COLOR_BUFFER_BIT);
            DrawScreenAlignedQuad();
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            
            glBindTexture(GL_TEXTURE_2D, hBlurTexture);
            glUniform2f(uniBlurAxis, 0.f, 1.2f);
            
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, vBlurFBO);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClear(GL_COLOR_BUFFER_BIT);
            DrawScreenAlignedQuad();
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            
            glBindTexture(GL_TEXTURE_2D, vBlurTexture);
            glUniform2f(uniBlurAxis, 1.2f, 0.f);
        }
        
        glUseProgramObjectARB(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
    }
}

GLuint OpenGLView::getSSAOTexture()
{
    if(ssaoSizeDiv > 0)
        return vBlurTexture;
    else
        return 0;
}

void OpenGLView::RenderFluid(FluidEntity* fluid)
{
    btVector3 position;
    btVector3 normal;
    fluid->GetSurface(normal, position);
    
    btTransform trans = GetViewTransform();
    glm::mat4 invProj= glm::inverse(projection);
    
    btVector3 transPlaneP = trans * position;
    btVector3 transPlaneN = trans.getBasis().inverse().transpose() * normal;
    btScalar eyeDepth = distanceFromCenteredPlane(transPlaneN, transPlaneP);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, getGBuffer()->getPositionTexture(0));
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sceneReflectionTexture);
    
    if(eyeDepth < 0) //above fluid surface
    {
        SetProjection();
        SetViewTransform();
        
        GLfloat c = 1.0/fluid->getFluid()->IOR;
        GLfloat R0 = 0.5f*powf((1.f-c)/(1.f+c), 2.f)*(1.f+powf((c*(1.f+c)-c*c)/(c*(1.f-c)+c*c), 2.f));
        
        glUseProgramObjectARB(fluidShader[0]);
        glUniform1i(uniFluidPosition[0], 0);
        glUniform1i(uniFluidScene[0], 1);
        glUniform1i(uniFluidReflection[0], 2);
        glUniform2f(uniFluidViewport[0], viewportWidth, viewportHeight);
        glUniformMatrix4fv(uniFluidIP[0], 1, GL_FALSE, glm::value_ptr(invProj));
        glUniform1f(uniFluidR0[0], R0);
        glUniform1f(uniFluidVisibility[0], 5.f);
        glUniform3f(uniFluidEyeSurfaceN[0], transPlaneN.x(), transPlaneN.y(), transPlaneN.z());
        glUniform3f(uniFluidEyeSurfaceP[0], transPlaneP.x(), transPlaneP.y(), transPlaneP.z());
        fluid->RenderSurface();
        glUseProgramObjectARB(0);
    }
    else if(fluid->IsInsideFluid(GetEyePosition()+GetLookingDirection()*near))
    {
        SetProjection();
        SetViewTransform();
        
        GLfloat c = 1.0/fluid->getFluid()->IOR;
        GLfloat R0 = 0.5f*powf((1.f-c)/(1.f+c), 2.f)*(1.f+powf((c*(1.f+c)-c*c)/(c*(1.f-c)+c*c), 2.f));
        
        glUseProgramObjectARB(fluidShader[1]);
        glUniform1i(uniFluidPosition[1], 0);
        glUniform1i(uniFluidScene[1], 1);
        glUniform1i(uniFluidReflection[1], 2);
        glUniform2f(uniFluidViewport[1], viewportWidth, viewportHeight);
        glUniformMatrix4fv(uniFluidIP[1], 1, GL_FALSE, glm::value_ptr(invProj));
        glUniform1f(uniFluidR0[1], R0);
        glUniform1f(uniFluidVisibility[1], 5.f);
        glUniform3f(uniFluidEyeSurfaceN[1], transPlaneN.x(), transPlaneN.y(), transPlaneN.z());
        glUniform3f(uniFluidEyeSurfaceP[1], transPlaneP.x(), transPlaneP.y(), transPlaneP.z());
        fluid->RenderSurface();
        glUseProgramObjectARB(0);
        
        //THIS WORKS BECAUSE I WRITE TO THE SCENE TEXTURE!
        glUseProgramObjectARB(fluidShader[2]);
        glUniform1i(uniFluidPosition[2], 0);
        glUniform1i(uniFluidScene[2], 1);
        glUniform1f(uniFluidVisibility[2], 5.f);
        glUniform2f(uniFluidViewport[2], viewportWidth, viewportHeight);
        glUniform3f(uniFluidEyeSurfaceN[2], transPlaneN.x(), transPlaneN.y(), transPlaneN.z());
        glUniform3f(uniFluidEyeSurfaceP[2], transPlaneP.x(), transPlaneP.y(), transPlaneP.z());
        fluid->RenderSurface();
        fluid->RenderVolume();
        glUseProgramObjectARB(0);
    }
    
    /*if(1) //fluid border crossed
    {
        glDisable(GL_DEPTH_TEST);
        SetupOrtho();
        glUseProgramObjectARB(fluidShader[3]);
        glUniform1i(uniFluidScene[3], 1);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -0.1f, 0.0f);
        glTexCoord2f(1, 0);
        glVertex3f(1.f, -0.1f, 0.0f);
        glTexCoord2f(1, 1);
        glVertex3f(1.f, 0.1f, 0.0f);
        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 0.1f, 0.0f);
        glEnd();
        glUseProgramObjectARB(0);
    }*/
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLView::Init()
{
    GLint compiled = 0;
    GLhandleARB vs = LoadShader(GL_VERTEX_SHADER, "saq.vert", &compiled);
    GLhandleARB fs = LoadShader(GL_FRAGMENT_SHADER, "ssao.frag", &compiled);
    ssaoShader = CreateProgramObject(vs, fs);
    LinkProgram(ssaoShader, &compiled);
    
    glUseProgramObjectARB(ssaoShader);
    uniSsaoNormal = glGetUniformLocationARB(ssaoShader, "texNormal");
    uniSsaoRandom = glGetUniformLocationARB(ssaoShader, "texRandom");
    uniSsaoViewport = glGetUniformLocationARB(ssaoShader, "viewport");
    uniSsaoRandomSize = glGetUniformLocationARB(ssaoShader, "random_size");
    uniSsaoRadius = glGetUniformLocationARB(ssaoShader, "radius");
    uniSsaoEpsilon = glGetUniformLocationARB(ssaoShader, "epsilon");
    uniSsaoFullOcclTh = glGetUniformLocationARB(ssaoShader, "full_occlusion_treshold");
    uniSsaoNoOcclTh = glGetUniformLocationARB(ssaoShader, "no_occlusion_treshold");
    uniSsaoOcclPower = glGetUniformLocationARB(ssaoShader, "occlusion_power");
    uniSsaoIP = glGetUniformLocationARB(ssaoShader, "inv_proj");
    uniSsaoP = glGetUniformLocationARB(ssaoShader, "proj");
    uniSsaoIVR = glGetUniformLocationARB(ssaoShader, "inv_rot");
    glUseProgramObjectARB(0);
    
    randomTexture = LoadInternalTexture("noise.png");
    
    fs = LoadShader(GL_FRAGMENT_SHADER, "blur.frag", &compiled);
    blurShader = CreateProgramObject(vs, fs);
    LinkProgram(blurShader, &compiled);
    
    glUseProgramObjectARB(blurShader);
    uniBlurNormal = glGetUniformLocationARB(blurShader, "texNormal");
    uniBlurSource = glGetUniformLocationARB(blurShader, "source");
    uniBlurViewport = glGetUniformLocationARB(blurShader, "viewport");
    uniBlurAxis = glGetUniformLocationARB(blurShader, "axis");
    uniBlurNormalFactor = glGetUniformLocationARB(blurShader, "normal_factor");
    uniBlurNormalPower = glGetUniformLocationARB(blurShader, "normal_power");
    uniBlurDepthPower = glGetUniformLocationARB(blurShader, "depth_power");
    uniBlurDistanceFactor = glGetUniformLocationARB(blurShader, "distance_factor");
    glUseProgramObjectARB(0);
    
    fs = LoadShader(GL_FRAGMENT_SHADER, "fluidBorder.frag", &compiled);
    fluidShader[3] = CreateProgramObject(vs, fs);
    LinkProgram(fluidShader[3], &compiled);
    
    glUseProgramObjectARB(fluidShader[3]);
    uniFluidScene[3] = glGetUniformLocationARB(fluidShader[3], "texScene");
    glUseProgramObjectARB(0);
    
    vs = LoadShader(GL_VERTEX_SHADER, "fluidSurface.vert", &compiled);
    fs = LoadShader(GL_FRAGMENT_SHADER, "aboveFluidSurface.frag", &compiled);
    fluidShader[0] = CreateProgramObject(vs, fs);
    LinkProgram(fluidShader[0], &compiled);
    
    glUseProgramObjectARB(fluidShader[0]);
    uniFluidReflection[0] = glGetUniformLocationARB(fluidShader[0], "texReflection");
    uniFluidScene[0] = glGetUniformLocationARB(fluidShader[0], "texScene");
    uniFluidPosition[0] = glGetUniformLocationARB(fluidShader[0], "texPosition");
    uniFluidViewport[0] = glGetUniformLocationARB(fluidShader[0], "viewport");
    uniFluidIP[0] = glGetUniformLocationARB(fluidShader[0], "invProj");
    uniFluidR0[0] = glGetUniformLocationARB(fluidShader[0], "R0");
    uniFluidEyeSurfaceN[0] = glGetUniformLocationARB(fluidShader[0], "eyeSurfaceNormal");
    uniFluidEyeSurfaceP[0] = glGetUniformLocationARB(fluidShader[0], "eyeSurfacePosition");
    uniFluidVisibility[0] = glGetUniformLocationARB(fluidShader[0], "visibility");
    glUseProgramObjectARB(0);

    fs = LoadShader(GL_FRAGMENT_SHADER, "belowFluidSurface.frag", &compiled);
    fluidShader[1] = CreateProgramObject(vs, fs);
    LinkProgram(fluidShader[1], &compiled);
    
    glUseProgramObjectARB(fluidShader[1]);
    uniFluidReflection[1] = glGetUniformLocationARB(fluidShader[1], "texReflection");
    uniFluidScene[1] = glGetUniformLocationARB(fluidShader[1], "texScene");
    uniFluidPosition[1] = glGetUniformLocationARB(fluidShader[1], "texPosition");
    uniFluidViewport[1] = glGetUniformLocationARB(fluidShader[1], "viewport");
    uniFluidIP[1] = glGetUniformLocationARB(fluidShader[1], "invProj");
    uniFluidR0[1] = glGetUniformLocationARB(fluidShader[1], "R0");
    uniFluidEyeSurfaceN[1] = glGetUniformLocationARB(fluidShader[1], "eyeSurfaceNormal");
    uniFluidEyeSurfaceP[1] = glGetUniformLocationARB(fluidShader[1], "eyeSurfacePosition");
    uniFluidVisibility[1] = glGetUniformLocationARB(fluidShader[1], "visibility");
    glUseProgramObjectARB(0);
    
    fs = LoadShader(GL_FRAGMENT_SHADER, "fluidVolume.frag", &compiled);
    fluidShader[2] = CreateProgramObject(vs, fs);
    LinkProgram(fluidShader[2], &compiled);
    
    glUseProgramObjectARB(fluidShader[2]);
    uniFluidScene[2] = glGetUniformLocationARB(fluidShader[2], "texScene");
    uniFluidPosition[2] = glGetUniformLocationARB(fluidShader[2], "texPosition");
    uniFluidEyeSurfaceN[2] = glGetUniformLocationARB(fluidShader[2], "eyeSurfaceNormal");
    uniFluidEyeSurfaceP[2] = glGetUniformLocationARB(fluidShader[2], "eyeSurfacePosition");
    uniFluidVisibility[2] = glGetUniformLocationARB(fluidShader[2], "visibility");
    uniFluidViewport[2] = glGetUniformLocationARB(fluidShader[2], "viewport");
    glUseProgramObjectARB(0);
}

void OpenGLView::Destroy()
{
    glDeleteTextures(1, &randomTexture);
    
    if(ssaoShader != NULL)
        glDeleteObjectARB(ssaoShader);
    
    if(blurShader != NULL)
        glDeleteObjectARB(blurShader);
    
    if(fluidShader[0] != NULL)
        glDeleteObjectARB(fluidShader[0]);
    
    if(fluidShader[1] != NULL)
        glDeleteObjectARB(fluidShader[1]);
    
    if(fluidShader[2] != NULL)
        glDeleteObjectARB(fluidShader[2]);
    
    if(fluidShader[3] != NULL)
        glDeleteObjectARB(fluidShader[2]);
}

void OpenGLView::SetTextureUnits(GLint normal, GLint random)
{
    normalTextureUnit = normal;
    randomTextureUnit = random;
}

GLuint OpenGLView::getRandomTexture()
{
    return randomTexture;
}