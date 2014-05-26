//
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLView.h"
#include "OpenGLSolids.h"
#include "OpenGLMaterial.h"
#include "GeometryUtil.h"
#include "OpenGLSun.h"
#include "SimulationApp.h"
#include "Console.h"

GLSLShader* OpenGLView::downsampleShader = NULL;
GLSLShader* OpenGLView::ssaoShader = NULL;
GLSLShader* OpenGLView::blurShader = NULL;
GLSLShader* OpenGLView::fluidShader[] = {NULL, NULL, NULL, NULL};
GLSLShader* OpenGLView::lightMeterShader = NULL;
GLSLShader* OpenGLView::tonemapShader = NULL;
GLint OpenGLView::randomTextureUnit = -1;
GLuint OpenGLView::randomTexture = 0;
GLuint OpenGLView::waveNormalTexture = 0;
GLint OpenGLView::positionTextureUnit = -1;
GLint OpenGLView::normalTextureUnit = -1;

OpenGLView::OpenGLView(GLint x, GLint y, GLint width, GLint height, GLfloat horizon, bool sao)
{
    viewportWidth = width;
    viewportHeight = height;
    originX = x;
    originY = y;
    fovx = 0.785f;
    active = false;
    ssaoSizeDiv = sao ? 2 : 0;
    gBuffer = new OpenGLGBuffer(viewportWidth, viewportHeight);
    far = UnitSystem::SetLength(horizon);
    near = 0.1f;
    
    glGenFramebuffersEXT(1, &sceneFBO);
    glGenRenderbuffersEXT(1, &sceneDepthBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, sceneFBO);
    
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, sceneDepthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, sceneDepthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
    glGenTextures(1, &finalTexture);
    glBindTexture(GL_TEXTURE_2D, finalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, FINAL_ATTACHMENT, GL_TEXTURE_2D, finalTexture, 0);
    
    glGenTextures(1, &sceneTexture);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, SCENE_ATTACHMENT, GL_TEXTURE_2D, sceneTexture, 0);
    
    if(OpenGLPipeline::getInstance()->isFluidRendered())
    {
        glGenTextures(1, &sceneReflectionTexture);
        glBindTexture(GL_TEXTURE_2D, sceneReflectionTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, REFLECTION_ATTACHMENT, GL_TEXTURE_2D, sceneReflectionTexture, 0);
        
        glGenTextures(1, &sceneRefractionTexture);
        glBindTexture(GL_TEXTURE_2D, sceneRefractionTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, REFRACTION_ATTACHMENT, GL_TEXTURE_2D, sceneRefractionTexture, 0);
    }
    
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Scene FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    glGenFramebuffersEXT(1, &lightMeterFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lightMeterFBO);
    
    glGenTextures(1, &lightMeterTexture);
    glBindTexture(GL_TEXTURE_2D, lightMeterTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1, 1, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, lightMeterTexture, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("Light meter FBO initialization failed!");
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    if(ssaoSizeDiv > 0)
    {
        glGenFramebuffersEXT(1, &ssaoFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ssaoFBO);
    
        glGenTextures(1, &ssaoTexture);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ssaoTexture, 0);
    
        GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
            cError("SAO FBO initialization failed!");
    
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
        glGenFramebuffersEXT(1, &hBlurFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hBlurFBO);
    
        glGenTextures(1, &hBlurTexture);
        glBindTexture(GL_TEXTURE_2D, hBlurTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, hBlurTexture, 0);
    
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
            cError("SAO horizontal blur FBO initialization failed!");
    
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
        glGenFramebuffersEXT(1, &vBlurFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, vBlurFBO);
    
        glGenTextures(1, &vBlurTexture);
        glBindTexture(GL_TEXTURE_2D, vBlurTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, vBlurTexture, 0);
    
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
            cError("SAO vertical blur FBO initialization failed!");
    
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
}

OpenGLView::~OpenGLView()
{
    glDeleteRenderbuffersEXT(1, &sceneDepthBuffer);
    glDeleteTextures(1, &finalTexture);
    glDeleteTextures(1, &sceneTexture);
    if(OpenGLPipeline::getInstance()->isFluidRendered())
    {
        glDeleteTextures(1, &sceneReflectionTexture);
        glDeleteTextures(1, &sceneRefractionTexture);
    }
    glDeleteFramebuffersEXT(1, &sceneFBO);
    glDeleteTextures(1, &lightMeterTexture);
    glDeleteFramebuffersEXT(1, &lightMeterFBO);
    
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

glm::mat4 OpenGLView::GetProjectionMatrix()
{
    return projection;
}

glm::mat4 OpenGLView::GetViewMatrix(const btTransform& viewTransform)
{
#ifdef BT_USE_DOUBLE_PRECISION
    double glmatrix[16];
    viewTransform.getOpenGLMatrix(glmatrix);
    glm::mat4 view((GLfloat)glmatrix[0],(GLfloat)glmatrix[1],(GLfloat)glmatrix[2],(GLfloat)glmatrix[3],
                   (GLfloat)glmatrix[4],(GLfloat)glmatrix[5],(GLfloat)glmatrix[6],(GLfloat)glmatrix[7],
                   (GLfloat)glmatrix[8],(GLfloat)glmatrix[9],(GLfloat)glmatrix[10],(GLfloat)glmatrix[11],
                   (GLfloat)glmatrix[12],(GLfloat)glmatrix[13],(GLfloat)glmatrix[14],(GLfloat)glmatrix[15]);
#else
    GLfloat glmatrix[16];
    viewTransform.getOpenGLMatrix(glmatrix);
    glm::mat4 view = glm::make_mat4(glmatrix);
#endif
    return view;
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

btVector3 OpenGLView::Ray(GLint x, GLint y)
{
    //translate point to view
    x -= originX;
    y -= originY;
    
    //check if point in view
    if((x < 0) || (x >= viewportWidth) || (y < 0) || (y >= viewportHeight))
        return btVector3(0,0,0);
    
    //calculate ray from point
    btVector3 rayFrom = GetEyePosition();
    btVector3 rayForward = GetLookingDirection() * far;
    btVector3 horizontal = rayForward.cross(GetUpDirection());
    horizontal.normalize();
    btVector3 vertical = horizontal.cross(rayForward);
    vertical.normalize();
    
    GLfloat tanFov = tanf(0.5f*fovx);
    horizontal *= 2.f * far * tanFov;
    vertical *= 2.f * far * tanFov;
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    vertical /= aspect;
    
    btVector3 rayToCenter = rayFrom + rayForward;
    btVector3 dH = horizontal * 1.f/(GLfloat)viewportWidth;
    btVector3 dV = vertical * 1.f/(GLfloat)viewportHeight;
    
    btVector3 rayTo = rayToCenter - 0.5f * horizontal + 0.5f * vertical;
    rayTo += btScalar(x) * dH;
    rayTo -= btScalar(y) * dV;
    
    return rayTo;
}

OpenGLGBuffer* OpenGLView::getGBuffer()
{
    return gBuffer;
}

GLuint OpenGLView::getSceneFBO()
{
    return sceneFBO;
}

GLuint OpenGLView::getFinalTexture()
{
    return finalTexture;
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

bool OpenGLView::hasSSAO()
{
    return ssaoSizeDiv > 0;
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
    gBuffer->SetClipPlane(NULL);
    
#ifdef BT_USE_DOUBLE_PRECISION
    glLoadMatrixd(openglTrans);
#else
    glLoadMatrixf(openglTrans);
#endif
}

void OpenGLView::SetReflectedViewTransform(FluidEntity* fluid)
{
    btVector3 normal, position;
    fluid->GetSurface(normal, position);
    btScalar eyeDepth = distanceFromCenteredPlane(normal, position - GetEyePosition());
    if(eyeDepth >= 0)
        normal = -normal;
    
    btTransform trans = GetReflectedViewTransform(fluid);
    btVector3 eyePosition = trans * position;
    btVector3 eyeNormal = trans.getBasis() * normal;
    
    double surface[4];
    surface[0] = eyeNormal.x();
    surface[1] = eyeNormal.y();
    surface[2] = eyeNormal.z();
    surface[3] = -eyeNormal.dot(eyePosition);
    
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //clip plane must be declared when modelview equals identity (otherwise it is transformed by inverse modelview)
    gBuffer->SetClipPlane(surface);
    
#ifdef BT_USE_DOUBLE_PRECISION
    glLoadMatrixd(openglTrans);
#else
    glLoadMatrixf(openglTrans);
#endif
}

void OpenGLView::SetRefractedViewTransform(FluidEntity* fluid)
{
    btVector3 normal, position;
    fluid->GetSurface(normal, position);
    btScalar eyeDepth = distanceFromCenteredPlane(normal, position - GetEyePosition());
    if(eyeDepth >= 0)
        normal = -normal;
    
    btTransform trans = GetRefractedViewTransform(fluid);
    btVector3 eyeNormal = trans.getBasis() * -normal;
    btVector3 eyePosition = trans * position;
    
    double surface[4];
    surface[0] = eyeNormal.x();
    surface[1] = eyeNormal.y();
    surface[2] = eyeNormal.z();
    surface[3] = -eyeNormal.dot(eyePosition);
    
    btTransform shift = btTransform(btQuaternion(0,0,0), btVector3(0,0,0));
    
    btScalar openglTrans[16];
    (trans * shift).getOpenGLMatrix(openglTrans);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //clip plane must be declared when modelview equals identity (otherwise it is transformed by inverse modelview)
    gBuffer->SetClipPlane(surface);
    
#ifdef BT_USE_DOUBLE_PRECISION
    glLoadMatrixd(openglTrans);
#else
    glLoadMatrixf(openglTrans);
#endif
}

btTransform OpenGLView::GetReflectedViewTransform(const FluidEntity* fluid)
{
    //get plane
    btVector3 normal, position;
    fluid->GetSurface(normal, position);
    btScalar eyeDepth = distanceFromCenteredPlane(normal, position - GetEyePosition());
    if(eyeDepth >= 0)
        normal = -normal;
    
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
    btTransform flip = btTransform::getIdentity();
    rotation = btMatrix3x3(1.0, 0.0, 0.0,
                           0.0, -1.0, 0.0,
                           0.0, 0.0, 1.0);
    flip.setBasis(rotation);
    
    //combine transforms
    trans = flip * trans * reflection;
    return trans;
}

btTransform OpenGLView::GetRefractedViewTransform(const FluidEntity* fluid)
{
    //get plane
    btVector3 normal, position;
    fluid->GetSurface(normal, position);
    btScalar eyeDepth = distanceFromCenteredPlane(normal, position - GetEyePosition());
    if(eyeDepth >= 0)
        normal = -normal;
    
    GLfloat planeEq[4];
    planeEq[0] = normal.x();
    planeEq[1] = normal.y();
    planeEq[2] = normal.z();
    planeEq[3] = -normal.dot(position);
    
    //get camera transform
    btVector3 dir = GetLookingDirection().normalize();
    btVector3 axis = dir.cross(normal);
    btScalar alpha = acos(normal.dot(-dir));
    btScalar beta = asin(axis.length()/fluid->getFluid()->IOR);
    axis = axis.normalize();
    
    btQuaternion rotation = btQuaternion(axis, alpha-beta);
    btTransform trans = GetViewTransform();
    
    //create reflection transform
    btTransform refraction = btTransform::getIdentity();
    refraction.setBasis(btMatrix3x3(rotation));
    
    //combine transforms
    trans = trans * refraction;
    
    return GetViewTransform();//trans;
}

void OpenGLView::ShowSceneTexture(SceneComponent sc, GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY)
{
    GLuint texture;
    
    switch (sc)
    {
        case NORMAL:
            texture = sceneTexture;
            break;
            
        case REFLECTED:
            texture = sceneReflectionTexture;
            break;
            
        case REFRACTED:
            texture = sceneRefractionTexture;
            break;
    }
    
    //Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewportWidth, 0, viewportHeight, 0.1f, 2.f);
    
	//Model setup
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, texture);
    
	// Render the quad
	glLoadIdentity();
	glTranslatef(x,-y,-1.0);
    
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex3f(0.0f,(float)viewportHeight, 0.0f);
	glTexCoord2f(0, 0);
	glVertex3f(0.0f, viewportHeight-sizeY, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(sizeX, viewportHeight-sizeY, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(sizeX, (float)viewportHeight, 0.0f);
	glEnd();
    
	glBindTexture(GL_TEXTURE_2D, 0);
    
	//Reset to the matrices
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void OpenGLView::RenderSSAO()
{
    if(hasSSAO())
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        //Draw SAO
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ssaoFBO);
        glViewport(0, 0, viewportWidth, viewportHeight);
        OpenGLSolids::SetupOrtho();
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glClear(GL_COLOR_BUFFER_BIT);
        
        GLfloat intensity = 0.5;
        GLfloat radius = 0.25;
        GLfloat projScale = 1.f/tanf(fovx/2.f)*(viewportWidth)/2.f;
        
        ssaoShader->Enable();
        ssaoShader->SetUniform("texRandom", randomTextureUnit);
        ssaoShader->SetUniform("texPosition", positionTextureUnit);
        ssaoShader->SetUniform("texNormal", normalTextureUnit);
        ssaoShader->SetUniform("radius", radius);
        ssaoShader->SetUniform("bias", 0.012f);
        ssaoShader->SetUniform("projScale", projScale);
        ssaoShader->SetUniform("intensityDivR6", (GLfloat)(intensity/pow(radius, 6.0)));
        ssaoShader->SetUniform("viewportSize", glm::vec2((GLfloat)viewportWidth, (GLfloat)viewportHeight));
        OpenGLSolids::DrawScreenAlignedQuad();
        ssaoShader->Disable();
        
        //Blur SAO
        glActiveTextureARB(GL_TEXTURE0_ARB + randomTextureUnit);
      
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        
        //Downsample SA0
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hBlurFBO);
        glViewport(0, 0, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        
        downsampleShader->Enable();
        downsampleShader->SetUniform("source", randomTextureUnit);
        downsampleShader->SetUniform("srcViewport", glm::vec2((GLfloat)viewportWidth, (GLfloat)viewportHeight));
        OpenGLSolids::DrawScreenAlignedQuad();
        downsampleShader->Disable();
       
        glBindTexture(GL_TEXTURE_2D, hBlurTexture);
        
        //Vertical blur
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, vBlurFBO);
        glViewport(0, 0, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        
        blurShader->Enable();
        blurShader->SetUniform("texSource", randomTextureUnit);
        blurShader->SetUniform("axis", glm::vec2(0.f, 1.f/(GLfloat)(viewportHeight/ssaoSizeDiv)));
        OpenGLSolids::DrawScreenAlignedQuad();
        blurShader->Disable();
        
        glBindTexture(GL_TEXTURE_2D, vBlurTexture);
        
        //Horizontal blur
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hBlurFBO);
        glViewport(0, 0, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        
        blurShader->Enable();
        blurShader->SetUniform("texSource", randomTextureUnit);
        blurShader->SetUniform("axis", glm::vec2(1.f/(GLfloat)(viewportHeight/ssaoSizeDiv), 0.f));
        OpenGLSolids::DrawScreenAlignedQuad();
        blurShader->Disable();
      
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        glPopAttrib();
    }
}

void OpenGLView::ShowAmbientOcclusion()
{
    //save current state
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib(GL_VIEWPORT_BIT);
    
    //Set projection and modelview
    OpenGLSolids::SetupOrtho();
        
    //Texture setup
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, getSSAOTexture());
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
        
    //Render the texture
    glViewport(0, 0, viewportWidth, viewportHeight);
    glColor4f(1.f,0.f,0.f,1.f);
    OpenGLSolids::DrawScreenAlignedQuad();
    
    //Reset
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

GLuint OpenGLView::getSSAOTexture()
{
    if(ssaoSizeDiv > 0)
        return hBlurTexture;
    else
        return 0;
}

void OpenGLView::RenderFluidSurface(FluidEntity* fluid, bool underwater)
{
    btVector3 position;
    btVector3 normal;
    fluid->GetSurface(normal, position);
    
    if(underwater)
        normal = -normal;
    
    btTransform trans = GetViewTransform();
    glm::mat4 invProj= glm::inverse(projection);
    
    btVector3 transPlaneP = trans * position;
    btVector3 transPlaneN = trans.getBasis() * normal;
    
    GLfloat c = 1.0/fluid->getFluid()->IOR;
    GLfloat R0 = 0.5f*powf((1.f-c)/(1.f+c), 2.f)*(1.f+powf((c*(1.f+c)-c*c)/(c*(1.f-c)+c*c), 2.f));
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    SetProjection();
    SetViewTransform();
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, getGBuffer()->getPositionTexture(0));
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sceneReflectionTexture);
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sceneRefractionTexture);
    
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, waveNormalTexture);
    
    fluidShader[0]->Enable();
    fluidShader[0]->SetUniform("texPosition", 0);
    fluidShader[0]->SetUniform("texReflection", 1);
    fluidShader[0]->SetUniform("texRefraction", 2);
    fluidShader[0]->SetUniform("texWaveNormal", 3);
    fluidShader[0]->SetUniform("viewport", glm::vec2((GLfloat)viewportWidth, (GLfloat)viewportHeight));
    fluidShader[0]->SetUniform("invProj", invProj);
    fluidShader[0]->SetUniform("R0", R0);
    fluidShader[0]->SetUniform("visibility", 5.0f);
    fluidShader[0]->SetUniform("eyeSurfaceNormal", glm::vec3((GLfloat)transPlaneN.x(), (GLfloat)transPlaneN.y(), (GLfloat)transPlaneN.z()));
    fluidShader[0]->SetUniform("eyeSurfacePosition", glm::vec3((GLfloat)transPlaneP.x(), (GLfloat)transPlaneP.y(), (GLfloat)transPlaneP.z()));
    fluidShader[0]->SetUniform("time", SimulationApp::getApp()->getSimulationManager()->getSimulationTime());
    fluid->RenderSurface();
    fluidShader[0]->Disable();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    btVector3 sunDirectionEye = trans * OpenGLSun::GetSunDirection();
    glm::vec3 sunDirEye((GLfloat)sunDirectionEye.x(), (GLfloat)sunDirectionEye.y(), (GLfloat)sunDirectionEye.z());
    
    fluidShader[1]->Enable();
    fluidShader[1]->SetUniform("texWaveNormal", 3);
    fluidShader[1]->SetUniform("viewport", glm::vec2((GLfloat)viewportWidth, (GLfloat)viewportHeight));
    fluidShader[1]->SetUniform("eyeSurfaceNormal", glm::vec3((GLfloat)transPlaneN.x(), (GLfloat)transPlaneN.y(), (GLfloat)transPlaneN.z()));
    fluidShader[1]->SetUniform("lightColor", OpenGLSun::GetSunColor());
    fluidShader[1]->SetUniform("lightDirection", sunDirEye);
    fluidShader[1]->SetUniform("time", SimulationApp::getApp()->getSimulationManager()->getSimulationTime());
    fluid->RenderSurface();
    fluidShader[1]->Disable();
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLView::RenderFluidVolume(FluidEntity* fluid)
{
    
}

void OpenGLView::RenderHDR()
{
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    
    OpenGLSolids::SetupOrtho();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    
    //matrix light metering
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lightMeterFBO);
    
    lightMeterShader->Enable();
    lightMeterShader->SetUniform("texHDR", 0);
    lightMeterShader->SetUniform("samples", glm::ivec2(24,16));
    OpenGLSolids::DrawScreenAlignedQuad();
    lightMeterShader->Disable();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    //hdr drawing
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lightMeterTexture);
    
    tonemapShader->Enable();
    tonemapShader->SetUniform("texHDR", 0);
    tonemapShader->SetUniform("texAverage", 1);
    OpenGLSolids::DrawScreenAlignedQuad();
    tonemapShader->Disable();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////// Static /////////////////////////////
void OpenGLView::Init()
{
    /////SAO - Screen-Space Ambient Obscurrance/////
    randomTexture = LoadInternalTexture("noise.png");
    
    ssaoShader = new GLSLShader("sao.frag");
    ssaoShader->AddUniform("texRandom", INT);
    ssaoShader->AddUniform("texPosition", INT);
    ssaoShader->AddUniform("texNormal", INT);
    ssaoShader->AddUniform("radius", FLOAT);
    ssaoShader->AddUniform("bias", FLOAT);
    ssaoShader->AddUniform("projScale", FLOAT);
    ssaoShader->AddUniform("intensityDivR6", FLOAT);
    ssaoShader->AddUniform("viewportSize", VEC2);
    
    blurShader = new GLSLShader("saoBlur.frag");
    blurShader->AddUniform("texSource", INT);
    blurShader->AddUniform("axis", VEC2);
    
    downsampleShader = new GLSLShader("saoDownsample.frag");
    downsampleShader->AddUniform("source", INT);
    downsampleShader->AddUniform("srcViewport", VEC2);
    
    //////Fluid//////
    waveNormalTexture = LoadInternalTexture("water.jpg");
    
    fluidShader[0] = new GLSLShader("aboveFluidSurface.frag");
    fluidShader[0]->AddUniform("texPosition", INT);
    fluidShader[0]->AddUniform("texReflection", INT);
    fluidShader[0]->AddUniform("texRefraction", INT);
    fluidShader[0]->AddUniform("viewport", VEC2);
    fluidShader[0]->AddUniform("invProj", MAT4);
    fluidShader[0]->AddUniform("R0", FLOAT);
    fluidShader[0]->AddUniform("eyeSurfaceNormal", VEC3);
    fluidShader[0]->AddUniform("eyeSurfacePosition", VEC3);
    fluidShader[0]->AddUniform("visibility", FLOAT);
    fluidShader[0]->AddUniform("texWaveNormal", INT);
    fluidShader[0]->AddUniform("time", FLOAT);
    
    fluidShader[1] = new GLSLShader("fluidSurfaceSun.frag");
    fluidShader[1]->AddUniform("texWaveNormal", INT);
    fluidShader[1]->AddUniform("viewport", VEC2);
    fluidShader[1]->AddUniform("eyeSurfaceNormal", VEC3);
    fluidShader[1]->AddUniform("lightDirection", VEC3);
    fluidShader[1]->AddUniform("lightColor", VEC4);
    fluidShader[1]->AddUniform("time", FLOAT);
    
    /////Tonemapping//////
    lightMeterShader = new GLSLShader("lightMeter.frag");
    lightMeterShader->AddUniform("texHDR", INT);
    lightMeterShader->AddUniform("samples", IVEC2);
    
    tonemapShader = new GLSLShader("tonemapping.frag");
    tonemapShader->AddUniform("texHDR", INT);
    tonemapShader->AddUniform("texAverage", INT);
}

void OpenGLView::Destroy()
{
    glDeleteTextures(1, &randomTexture);
    
    if(ssaoShader != NULL)
        delete ssaoShader;
    
    if(blurShader != NULL)
        delete blurShader;
    
    if(downsampleShader != NULL)
        delete downsampleShader;
    
    if(lightMeterShader != NULL)
        delete lightMeterShader;
    
    if(tonemapShader != NULL)
        delete tonemapShader;
    
    if(fluidShader[0] != NULL)
        delete fluidShader[0];
    
    if(fluidShader[1] != NULL)
        delete fluidShader[1];
    
    if(fluidShader[2] != NULL)
        delete fluidShader[2];
    
    if(fluidShader[3] != NULL)
        delete fluidShader[3];
}

void OpenGLView::SetTextureUnits(GLint position, GLint normal, GLint random)
{
    positionTextureUnit = position;
    normalTextureUnit = normal;
    randomTextureUnit = random;
}

GLuint OpenGLView::getRandomTexture()
{
    return randomTexture;
}