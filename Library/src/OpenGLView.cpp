//
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLView.h"
#include "GeometryUtil.hpp"
#include "OpenGLSun.h"
#include "SimulationApp.h"
#include "Console.h"
#include "SystemUtil.hpp"

GLSLShader* OpenGLView::downsampleShader = NULL;
GLSLShader* OpenGLView::ssaoShader = NULL;
GLSLShader* OpenGLView::blurShader = NULL;
GLSLShader* OpenGLView::lightMeterShader = NULL;
GLSLShader* OpenGLView::tonemapShader = NULL;
GLint OpenGLView::randomTextureUnit = -1;
GLuint OpenGLView::randomTexture = 0;
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
    far = UnitSystem::SetLength(horizon);
    near = 0.1f;
	activePostprocessTexture = 0;
    
	int samples = 4;
	
	//----Multisampled rendering----
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    
	glGenRenderbuffers(1, &renderDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, renderDepthStencil);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glGenTextures(1, &renderColorTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderColorTex);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB16F, viewportWidth, viewportHeight, GL_TRUE); //0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderColorTex, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Render FBO initialization failed!");
    
	//----Light metering (automatic exposure)----
    glGenFramebuffers(1, &lightMeterFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lightMeterFBO);
    
    glGenTextures(1, &lightMeterTex);
    glBindTexture(GL_TEXTURE_2D, lightMeterTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 2, 2, 0, GL_RGB, GL_FLOAT, NULL); //Distribute work to 4 parallel threads
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Use hardware linear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightMeterTex, 0);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Light meter FBO initialization failed!");
    
	//----Non-multisampled postprocessing----
	glGenFramebuffers(1, &postprocessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
	
	glGenTextures(2, postprocessTex);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessTex[0], 0);
	
	glBindTexture(GL_TEXTURE_2D, postprocessTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, postprocessTex[1], 0);
	
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Postprocess FBO initialization failed!");
    
	//----SSAO----
    if(ssaoSizeDiv > 0)
    {
        glGenFramebuffers(1, &ssaoFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    
        glGenTextures(1, &ssaoTexture);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
    
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
            cError("SSAO FBO initialization failed!");
    
        glGenFramebuffers(1, &blurFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    
        glGenTextures(1, &hBlurTexture);
        glBindTexture(GL_TEXTURE_2D, hBlurTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hBlurTexture, 0);
    
        glGenTextures(1, &vBlurTexture);
        glBindTexture(GL_TEXTURE_2D, vBlurTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, vBlurTexture, 0);
    
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
            cError("SSAO blur FBO initialization failed!");
    }
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLView::~OpenGLView()
{
    glDeleteRenderbuffers(1, &renderDepthStencil);
    glDeleteTextures(2, postprocessTex);
    glDeleteTextures(1, &renderColorTex);
    glDeleteTextures(1, &lightMeterTex);
	glDeleteFramebuffers(1, &renderFBO);
    glDeleteFramebuffers(1, &postprocessFBO);
	glDeleteFramebuffers(1, &lightMeterFBO);
    
    if(ssaoSizeDiv > 0)
    {
        glDeleteTextures(1, &ssaoTexture);
        glDeleteTextures(1, &hBlurTexture);
        glDeleteTextures(1, &vBlurTexture);
        glDeleteFramebuffers(1, &renderFBO);
        glDeleteFramebuffers(1, &ssaoFBO);
        glDeleteFramebuffers(1, &blurFBO);
    }
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

glm::mat4 OpenGLView::GetViewMatrix()
{
	return GetViewTransform();
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
	glm::vec3 _eye = GetEyePosition();
	glm::vec3 _lookingDir = GetLookingDirection();
	glm::vec3 _up = GetUpDirection();
	
    btVector3 rayFrom(_eye.x, _eye.y, _eye.z);
    btVector3 rayForward = btVector3(_lookingDir.x, _lookingDir.y, _lookingDir.z) * far;
    btVector3 horizontal = rayForward.cross(btVector3(_up.x, _up.y, _up.z));
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

GLuint OpenGLView::getRenderFBO()
{
    return renderFBO;
}

GLuint OpenGLView::getFinalTexture()
{
    return postprocessTex[activePostprocessTexture];
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
}

void OpenGLView::SetViewport()
{
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void OpenGLView::SetProjection()
{
	OpenGLContent::getInstance()->SetProjectionMatrix(projection);
}

void OpenGLView::SetViewTransform()
{
    OpenGLContent::getInstance()->SetViewMatrix(GetViewTransform());
}

void OpenGLView::ShowSceneTexture(SceneComponent sc, GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY)
{
    GLuint texture;
    
    switch (sc)
    {
		default:
        case NORMAL:
            texture = renderColorTex;
            break;
    }
    
    OpenGLContent::getInstance()->DrawTexturedQuad(x, y, sizeX, sizeY, texture);
}

void OpenGLView::RenderSSAO()
{
    if(hasSSAO())
    {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        //Draw SAO
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glViewport(0, 0, viewportWidth, viewportHeight);
        
        btVector3 min, max;
        SimulationApp::getApp()->getSimulationManager()->getWorldAABB(min, max);
        GLfloat meanDimension = ((max[0] - min[0]) + (max[1] - min[1]) + (max[2] - min[2]))/3.f;
        GLfloat radius = meanDimension * 0.2;
        GLfloat intensity = 1.0;
        GLfloat projScale = 1.f/tanf(fovx/2.f)*(viewportWidth)/2.f;
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        ssaoShader->Use();
        ssaoShader->SetUniform("texRandom", randomTextureUnit);
        ssaoShader->SetUniform("texPosition", positionTextureUnit);
        ssaoShader->SetUniform("texNormal", normalTextureUnit);
        ssaoShader->SetUniform("radius", radius);
        ssaoShader->SetUniform("bias", 0.012f);
        ssaoShader->SetUniform("projScale", projScale);
        ssaoShader->SetUniform("intensityDivR6", (GLfloat)(intensity/pow(radius, 6.0)));
        ssaoShader->SetUniform("viewportSize", glm::vec2((GLfloat)viewportWidth, (GLfloat)viewportHeight));
        OpenGLContent::getInstance()->DrawSAQ();
        
        //Blur SAO
        glActiveTexture(GL_TEXTURE0 + randomTextureUnit);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        
        //Downsample SA0
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
        glViewport(0, 0, viewportWidth/ssaoSizeDiv, viewportHeight/ssaoSizeDiv);
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        downsampleShader->Use();
        downsampleShader->SetUniform("source", randomTextureUnit);
        downsampleShader->SetUniform("srcViewport", glm::vec2((GLfloat)viewportWidth, (GLfloat)viewportHeight));
        OpenGLContent::getInstance()->DrawSAQ();
       
        blurShader->Use();
        blurShader->SetUniform("source", randomTextureUnit);
        
        for(unsigned int i = 0; i < 2; ++i)
        {
            //Vertical blur
            glBindTexture(GL_TEXTURE_2D, hBlurTexture);
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            blurShader->SetUniform("texelOffset", glm::vec2(0.f, 1.f/(GLfloat)(viewportHeight/ssaoSizeDiv)));
            OpenGLContent::getInstance()->DrawSAQ();
            
            //Horizontal blur
            glBindTexture(GL_TEXTURE_2D, vBlurTexture);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            blurShader->SetUniform("texelOffset", glm::vec2(1.f/(GLfloat)(viewportWidth/ssaoSizeDiv), 0.f));
            OpenGLContent::getInstance()->DrawSAQ();
        }
        
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
		
		glViewport(0, 0, viewportWidth, viewportHeight);
    }
}

void OpenGLView::ShowAmbientOcclusion(GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY)
{
	OpenGLContent::getInstance()->DrawTexturedQuad(x, y, sizeX, sizeY, getSSAOTexture());
}

GLuint OpenGLView::getSSAOTexture()
{
    if(ssaoSizeDiv > 0)
        return hBlurTexture;
    else
        return 0;
}

void OpenGLView::RenderHDR(GLuint destinationFBO)
{
	//Blit multisampled to non-multisampled texture
	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFBO);
	glBlitFramebuffer(0, 0, viewportWidth, viewportHeight, 0, 0, viewportWidth, viewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
	
    //matrix light metering
    glBindFramebuffer(GL_FRAMEBUFFER, lightMeterFBO);
    lightMeterShader->Use();
    lightMeterShader->SetUniform("texHDR", 0);
    lightMeterShader->SetUniform("samples", glm::ivec2(128,128));
    OpenGLContent::getInstance()->DrawSAQ();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //hdr drawing
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lightMeterTex);
    
    glBindFramebuffer(GL_FRAMEBUFFER, destinationFBO);
    tonemapShader->Use();
    tonemapShader->SetUniform("texHDR", 0);
    tonemapShader->SetUniform("texAverage", 1);
    OpenGLContent::getInstance()->DrawSAQ();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
	glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////// Static /////////////////////////////
void OpenGLView::Init()
{
    /////SAO - Screen-Space Ambient Obscurrance/////
    randomTexture = OpenGLContent::LoadInternalTexture("noise.png");
    
    ssaoShader = new GLSLShader("sao.frag");
    ssaoShader->AddUniform("texRandom", INT);
    ssaoShader->AddUniform("texPosition", INT);
    ssaoShader->AddUniform("texNormal", INT);
    ssaoShader->AddUniform("radius", FLOAT);
    ssaoShader->AddUniform("bias", FLOAT);
    ssaoShader->AddUniform("projScale", FLOAT);
    ssaoShader->AddUniform("intensityDivR6", FLOAT);
    ssaoShader->AddUniform("viewportSize", VEC2);
    
    downsampleShader = new GLSLShader("saoDownsample.frag");
    downsampleShader->AddUniform("source", INT);
    downsampleShader->AddUniform("srcViewport", VEC2);
    
    blurShader = new GLSLShader("saoBlur.frag", "gaussianBlur.vert");
    blurShader->AddUniform("source", INT);
    blurShader->AddUniform("texelOffset", VEC2);
    
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
