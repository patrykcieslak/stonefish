//
//  OpenGLCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLCamera.h"

#include <random>
#include "core/GraphicalSimulationApp.h"
#include "core/Console.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "entities/SolidEntity.h"

namespace sf
{

GLSLShader** OpenGLCamera::depthAwareBlurShader = NULL;
GLSLShader* OpenGLCamera::lightMeterShader = NULL;
GLSLShader* OpenGLCamera::tonemapShader = NULL;
GLSLShader** OpenGLCamera::depthLinearizeShader = NULL;
GLSLShader* OpenGLCamera::aoDeinterleaveShader = NULL;
GLSLShader** OpenGLCamera::aoCalcShader = NULL;
GLSLShader* OpenGLCamera::aoReinterleaveShader = NULL;
GLSLShader** OpenGLCamera::aoBlurShader = NULL;
GLSLShader** OpenGLCamera::ssrShader = NULL;

OpenGLCamera::OpenGLCamera(GLint x, GLint y, GLint width, GLint height, GLfloat horizon, GLuint spp, bool ao) : OpenGLView(x, y, width, height)
{
    fovx = 0.785f;
    aoFactor = ao ? 1 : 0;
	samples = spp < 1 ? 1 : (spp > 8 ? 8 : spp);
    far = horizon;
    near = 0.1f;
	activePostprocessTexture = 0;
    exposureComp = 0.f;
    
    if(!GLEW_VERSION_4_3)
        aoFactor = 0;
    
    if(!((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getRenderSettings().msaa)
        samples = 1;
    
	//----Geometry rendering----
	//Normal render buffer
	if(samples > 1) //MSAA
	{
        glGenTextures(1, &renderColorTex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderColorTex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB16F, viewportWidth, viewportHeight, GL_FALSE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		
		glGenTextures(1, &renderViewNormalTex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderViewNormalTex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, viewportWidth, viewportHeight, GL_FALSE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		
		glGenTextures(1, &renderDepthStencilTex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderDepthStencilTex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight, GL_FALSE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}
	else
	{
		glGenTextures(1, &renderColorTex);
		glBindTexture(GL_TEXTURE_2D, renderColorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewportWidth, viewportHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glGenTextures(1, &renderViewNormalTex);
		glBindTexture(GL_TEXTURE_2D, renderViewNormalTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glGenTextures(1, &renderDepthStencilTex);
		glBindTexture(GL_TEXTURE_2D, renderDepthStencilTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderColorTex, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, renderViewNormalTex, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, renderDepthStencilTex, 0);
	
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Render FBO initialization failed!");
		
	//----Light metering (automatic exposure)----
    glGenFramebuffers(1, &lightMeterFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lightMeterFBO);
    
    glGenTextures(1, &lightMeterTex);
    glBindTexture(GL_TEXTURE_2D, lightMeterTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 2, 2, 0, GL_RGB, GL_FLOAT, NULL); //Distribute work to 4 parallel threads
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewportWidth, viewportHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessTex[0], 0);
	
	glBindTexture(GL_TEXTURE_2D, postprocessTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, postprocessTex[1], 0);
	
    glGenTextures(1, &postprocessStencilTex);
    glBindTexture(GL_TEXTURE_2D, postprocessStencilTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, postprocessStencilTex, 0);
    
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Postprocess FBO initialization failed!");
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//Linear depth
	glGenTextures(2, linearDepthTex);
    //Front face
	glBindTexture(GL_TEXTURE_2D, linearDepthTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth, viewportHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture (GL_TEXTURE_2D, 0);
    //Back face
    glBindTexture(GL_TEXTURE_2D, linearDepthTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth, viewportHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture (GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &linearDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, linearDepthFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, linearDepthTex[0], 0);
		
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		cError("LinearDepth FBO initialization failed!");
	
	//----HBAO----
    if(aoFactor > 0)
    {
		//Deinterleaved results
		GLint swizzle[4] = {GL_RED,GL_GREEN,GL_ZERO,GL_ZERO};
		
		glGenTextures(1, &aoResultTex);
		glBindTexture(GL_TEXTURE_2D, aoResultTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, viewportWidth, viewportHeight);
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &aoBlurTex);
		glBindTexture(GL_TEXTURE_2D, aoBlurTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, viewportWidth, viewportHeight);
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &aoFinalFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, aoFinalFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoResultTex, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, aoBlurTex, 0);
		
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
			cError("AOFinal FBO initialization failed!");
		
		//Interleaved rendering
		int quarterWidth  = ((viewportWidth+3)/4);
		int quarterHeight = ((viewportHeight+3)/4);

		glGenTextures(1, &aoDepthArrayTex);
		glBindTexture (GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		glGenTextures(HBAO_RANDOM_ELEMENTS, aoDepthViewTex);
		for(int i=0; i<HBAO_RANDOM_ELEMENTS; ++i)
		{
			glTextureView(aoDepthViewTex[i], GL_TEXTURE_2D, aoDepthArrayTex, GL_R32F, 0, 1, i, 1);
			glBindTexture(GL_TEXTURE_2D, aoDepthViewTex[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		
		glGenTextures(1, &aoResultArrayTex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, aoResultArrayTex);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RG16F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		GLenum drawbuffers[NUM_MRT];
		for(int layer = 0; layer < NUM_MRT; ++layer)
			drawbuffers[layer] = GL_COLOR_ATTACHMENT0 + layer;

		glGenFramebuffers(1, &aoDeinterleaveFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, aoDeinterleaveFBO);
		glDrawBuffers(NUM_MRT, drawbuffers);
		
		glGenFramebuffers(1, &aoCalcFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, aoCalcFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoResultArrayTex, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glGenBuffers(1, &aoDataUBO);
		glNamedBufferStorageEXT(aoDataUBO, sizeof(AOData), NULL, GL_DYNAMIC_STORAGE_BIT);
		
		//Generate random data
		std::mt19937 rng((unsigned int)GetTimeInMicroseconds());
		GLfloat numDir = 8; //Keep in sync with GLSL shader!!!
		GLfloat rngMax = (GLfloat)rng.max() + 1.f; 
		
		for(unsigned int i=0; i<HBAO_RANDOM_ELEMENTS; ++i)
		{
			GLfloat rand1 = (GLfloat)rng()/rngMax; //random in [0,1)
			GLfloat rand2 = (GLfloat)rng()/rngMax; //random in [0,1)

			//Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
			GLfloat angle = 2.f * M_PI * rand1 / numDir;
			aoData.jitters[i].x = cosf(angle);
			aoData.jitters[i].y = sinf(angle);
			aoData.jitters[i].z = rand2;
			aoData.jitters[i].w = 0;
			
			aoData.float2Offsets[i] = glm::vec4((GLfloat)(i%4) + 0.5f, (GLfloat)(i/4) + 0.5f, 0.0, 0.0);
		}
    }
}

OpenGLCamera::~OpenGLCamera()
{
    glDeleteTextures(1, &renderColorTex);
	glDeleteTextures(1, &renderViewNormalTex);
	glDeleteTextures(1, &renderDepthStencilTex);
	glDeleteTextures(1, &lightMeterTex);
	glDeleteTextures(2, linearDepthTex);
    glDeleteTextures(2, postprocessTex);
    glDeleteTextures(1, &postprocessStencilTex);
	
	glDeleteFramebuffers(1, &renderFBO);
	glDeleteFramebuffers(1, &postprocessFBO);
	glDeleteFramebuffers(1, &lightMeterFBO);
    glDeleteFramebuffers(1, &linearDepthFBO);
    
    if(aoFactor > 0)
    {
		glDeleteTextures(1, &aoResultTex);
		glDeleteTextures(1, &aoBlurTex);
		glDeleteTextures(1, &aoDepthArrayTex);
		glDeleteTextures(1, &aoResultArrayTex);
		glDeleteTextures(HBAO_RANDOM_ELEMENTS, aoDepthViewTex);
	
		glDeleteFramebuffers(1, &aoFinalFBO);
		glDeleteFramebuffers(1, &aoDeinterleaveFBO);
		glDeleteFramebuffers(1, &aoCalcFBO);
		
		glDeleteBuffers(1, &aoDataUBO);
    }
}

glm::mat4 OpenGLCamera::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLCamera::GetInfiniteProjectionMatrix() const
{
    glm::mat4 infProj = projection;
	infProj[2][2] = -1.f;
	infProj[2][3] = -1.f;
	infProj[3][2] = -2.f*near;
    return infProj;
}

GLfloat OpenGLCamera::GetFOVX() const
{
	return fovx;
}

GLfloat OpenGLCamera::GetFOVY() const
{
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    return fovx/aspect;
}

GLfloat OpenGLCamera::GetNearClip()
{
    return near;
}

GLfloat OpenGLCamera::GetFarClip()
{
    return far;
}

glm::vec3 OpenGLCamera::Ray(GLint x, GLint y)
{
    //translate point to view
    x -= originX;
    y -= originY;
    
    //check if point in view
    if((x < 0) || (x >= viewportWidth) || (y < 0) || (y >= viewportHeight))
        return glm::vec3(0);
    
    //calculate ray from point
	glm::vec3 _eye = GetEyePosition();
	glm::vec3 _lookingDir = GetLookingDirection();
	glm::vec3 _up = GetUpDirection();
	
    glm::vec3 rayFrom = _eye;
    glm::vec3 rayForward = _lookingDir * far;
    glm::vec3 horizontal = glm::normalize(glm::cross(rayForward, _up));
    glm::vec3 vertical = glm::normalize(glm::cross(horizontal, rayForward));
    
    GLfloat tanFov = tanf(0.5f*fovx);
    horizontal *= 2.f * far * tanFov;
    vertical *= 2.f * far * tanFov;
    GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    vertical /= aspect;
    
    glm::vec3 rayToCenter = rayFrom + rayForward;
    glm::vec3 dH = horizontal * 1.f/(GLfloat)viewportWidth;
    glm::vec3 dV = vertical * 1.f/(GLfloat)viewportHeight;
    
    glm::vec3 rayTo = rayToCenter - 0.5f * horizontal + 0.5f * vertical;
    rayTo += Scalar(x) * dH;
    rayTo -= Scalar(y) * dV;
    
    return rayTo;
}
    
void OpenGLCamera::setExposureCompensation(GLfloat ec)
{
    exposureComp = ec;
}
    
GLfloat OpenGLCamera::getExposureCompensation()
{
    return exposureComp;
}

GLuint OpenGLCamera::getColorTexture()
{
    return renderColorTex;
}
    
GLuint OpenGLCamera::getFinalTexture()
{
    return postprocessTex[activePostprocessTexture];
}

bool OpenGLCamera::hasAO()
{
    return aoFactor > 0;
}

void OpenGLCamera::SetProjection()
{
	((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetProjectionMatrix(projection);
}

void OpenGLCamera::SetViewTransform()
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetViewMatrix(GetViewMatrix());
}

void OpenGLCamera::ShowSceneTexture(glm::vec4 rect)
{
    if(samples>1)
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderColorTex, glm::ivec2(viewportWidth, viewportHeight));
    else
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderColorTex);
}

void OpenGLCamera::ShowLinearDepthTexture(glm::vec4 rect, bool frontFace)
{
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, frontFace ? linearDepthTex[0] : linearDepthTex[1]);
}

void OpenGLCamera::ShowViewNormalTexture(glm::vec4 rect)
{
	if(samples>1)
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex, glm::ivec2(viewportWidth, viewportHeight));
	else
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex);
}

void OpenGLCamera::ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index)
{
	if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoDepthArrayTex, index);
}

void OpenGLCamera::ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index)
{
	if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoResultArrayTex, index);
}

void OpenGLCamera::GenerateLinearDepth(int sampleId, bool frontFace)
{
	glBindFramebuffer(GL_FRAMEBUFFER, linearDepthFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frontFace ? linearDepthTex[0] : linearDepthTex[1], 0);
    glViewport(0, 0, viewportWidth, viewportHeight);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    
	if(samples>1)
	{
		depthLinearizeShader[1]->Use();
		depthLinearizeShader[1]->SetUniform("clipInfo", glm::vec4(near*far, near-far, far, 1.f));
		depthLinearizeShader[1]->SetUniform("sampleIndex", sampleId);
		depthLinearizeShader[1]->SetUniform("texDepth", TEX_POSTPROCESS1);
			
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderDepthStencilTex);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}
	else
	{
		depthLinearizeShader[0]->Use();
		depthLinearizeShader[0]->SetUniform("clipInfo", glm::vec4(near*far, near-far, far, 1.f));
		depthLinearizeShader[0]->SetUniform("texDepth", TEX_POSTPROCESS1);
		
        glBindTexture(GL_TEXTURE_2D, renderDepthStencilTex);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint OpenGLCamera::getLinearDepthTexture(bool frontFace)
{
    return frontFace ? linearDepthTex[0] : linearDepthTex[1];
}

void OpenGLCamera::DrawAO(GLfloat intensity)
{
    if(hasAO())
    {
		//Prepare data and set parameters
		int quarterWidth  = ((viewportWidth+3)/4);
		int quarterHeight = ((viewportHeight+3)/4);
		glm::mat4 proj = GetProjectionMatrix();
		glm::vec4 projInfo(
							2.0f/proj[0].x,
							2.0f/proj[1].y,
							-(1.f-proj[0].z)/proj[0].x,
							-(1.f+proj[1].z)/proj[1].y
							);
						  
		glm::vec2 invFullRes(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight);
		glm::vec2 invQuarterRes(1.f/(GLfloat)quarterWidth, 1.f/(GLfloat)quarterHeight);
		GLfloat projScale = (GLfloat)viewportHeight/(tanf(GetFOVY() * 0.5f) * 2.0f);
		GLfloat R = 0.5f;
		
		aoData.projInfo = projInfo;
		aoData.R2 = R * R;
		aoData.NegInvR2 = -1.f/aoData.R2;
		aoData.RadiusToScreen = R * 0.5f * projScale;
		aoData.PowExponent = intensity < 0.f ? 0.f : intensity; //intensity
		aoData.NDotVBias = 0.01f;  //<0,1>
		aoData.AOMultiplier = 1.f/(1.f-aoData.NDotVBias);
		aoData.InvQuarterResolution = invQuarterRes;
		aoData.InvFullResolution = invFullRes;
		
		GLfloat blurSharpness = 40.0f;
		
		//For all samples
		for(unsigned int n=0; n<samples; ++n)
		{
			GenerateLinearDepth(n, true);
			((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray(); //Previous function unbinds vertex array
		
			//Deinterleave
			glBindFramebuffer(GL_FRAMEBUFFER, aoDeinterleaveFBO);
			glViewport(0, 0, quarterWidth, quarterHeight);

			aoDeinterleaveShader->Use();
			aoDeinterleaveShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
		
            glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
            glBindTexture(GL_TEXTURE_2D, linearDepthTex[0]);
			for(int i=0; i<HBAO_RANDOM_ELEMENTS; i+=NUM_MRT)
			{
				aoDeinterleaveShader->SetUniform("info", glm::vec4(float(i % 4) + 0.5f, float(i / 4) + 0.5f, invFullRes.x, invFullRes.y));
			
				for(int layer = 0; layer < NUM_MRT; ++layer)
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, aoDepthViewTex[i+layer], 0);
			
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			
			//Calculate HBAO
			glBindFramebuffer(GL_FRAMEBUFFER, aoCalcFBO);
			glViewport(0, 0, quarterWidth, quarterHeight);
			
			if(samples>1)
			{
				aoCalcShader[1]->Use();
				aoCalcShader[1]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
				aoCalcShader[1]->SetUniform("texViewNormal", TEX_POSTPROCESS2);
				aoCalcShader[1]->SetUniform("sampleIndex", (int)n);
		
                glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderViewNormalTex);
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, aoDataUBO);
				glNamedBufferSubDataEXT(aoDataUBO, 0, sizeof(AOData), &aoData);
                glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
                glBindTexture(GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
				glDrawArrays(GL_TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
                glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			}
			else
			{
				aoCalcShader[0]->Use();
				aoCalcShader[0]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
				aoCalcShader[0]->SetUniform("texViewNormal", TEX_POSTPROCESS2);
		
                glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
                glBindTexture(GL_TEXTURE_2D, renderViewNormalTex);
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, aoDataUBO);
				glNamedBufferSubDataEXT(aoDataUBO, 0, sizeof(AOData), &aoData);
                glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
                glBindTexture(GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
				glDrawArrays(GL_TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
                glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, viewportWidth, viewportHeight);
		
			//Reinterleave
			glBindFramebuffer(GL_FRAMEBUFFER, aoFinalFBO);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glViewport(0, 0, viewportWidth, viewportHeight);
			aoReinterleaveShader->Use();
			aoReinterleaveShader->SetUniform("texResultArray", TEX_POSTPROCESS1);
		
            glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
            glBindTexture(GL_TEXTURE_2D_ARRAY, aoResultArrayTex);
			glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		
			//Blur
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			aoBlurShader[0]->Use();
			aoBlurShader[0]->SetUniform("sharpness", blurSharpness);
			aoBlurShader[0]->SetUniform("invResolutionDirection", glm::vec2(1.f/(GLfloat)viewportWidth, 0));
			aoBlurShader[0]->SetUniform("texSource", TEX_POSTPROCESS1);
		
            glBindTexture(GL_TEXTURE_2D, aoResultTex);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindTexture(GL_TEXTURE_2D, 0);

			//Final output to main fbo
			glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glDepthMask(GL_FALSE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			//glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_DST_ALPHA);
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);
            
			if(samples>1)
			{
				glEnable(GL_SAMPLE_MASK);
				glSampleMaski(0, 1<<n);
			}
	
			aoBlurShader[1]->Use();
			aoBlurShader[1]->SetUniform("sharpness", blurSharpness);
			aoBlurShader[1]->SetUniform("invResolutionDirection", glm::vec2(0, 1.f/(GLfloat)viewportHeight));
			aoBlurShader[1]->SetUniform("texSource", TEX_POSTPROCESS1);
		
            glBindTexture(GL_TEXTURE_2D, aoBlurTex);
			glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindTexture(GL_TEXTURE_2D, 0);
		
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			glDisable(GL_SAMPLE_MASK);
			glSampleMaski(0, ~0);
		}
		glBindVertexArray(0);
		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	}
}

void OpenGLCamera::DrawSSR()
{
    //Assumes that postprocessing FBO is selected
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[1]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS3);
    glBindTexture(GL_TEXTURE_2D, getLinearDepthTexture(true));
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS4);
    glBindTexture(GL_TEXTURE_2D, getLinearDepthTexture(false));
    
    GLfloat sx = viewportWidth/2.f;
    GLfloat sy = viewportHeight/2.f;
    
    glm::mat4 proj = GetProjectionMatrix();
    
    glm::mat4 projPix = glm::transpose(glm::mat4(sx,   0,   0,  sx,
                                                  0, sy,   0,  sy,
                                                  0,   0, 1.f,   0,
                                                  0,   0,   0, 1.f)) * proj;
    
    glm::vec4 projInfo(
                       2.0f/proj[0].x,
                       2.0f/proj[1].y,
                       -(1.f-proj[0].z)/proj[0].x,
                       -(1.f+proj[1].z)/proj[1].y
                       );
    
    ssrShader[0]->Use();
    ssrShader[0]->SetUniform("texColor", TEX_POSTPROCESS1);
    ssrShader[0]->SetUniform("texViewNormal", TEX_POSTPROCESS2);
    ssrShader[0]->SetUniform("texLinearDepth", TEX_POSTPROCESS3);
    ssrShader[0]->SetUniform("texLinearBackfaceDepth", TEX_POSTPROCESS4);
    ssrShader[0]->SetUniform("P", projPix);
    ssrShader[0]->SetUniform("invP", glm::inverse(proj));
    ssrShader[0]->SetUniform("projInfo", projInfo);
    ssrShader[0]->SetUniform("viewportSize", glm::vec2(viewportWidth, viewportHeight));
    ssrShader[0]->SetUniform("invViewportSize", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight));
    ssrShader[0]->SetUniform("near", near);
    ssrShader[0]->SetUniform("far", far);
    ssrShader[0]->SetUniform("maxIterations", 100);
    ssrShader[0]->SetUniform("maxBinarySearchIterations", 10);
    ssrShader[0]->SetUniform("pixelZSize", 1.0f);
    ssrShader[0]->SetUniform("pixelStride", 1.f);
    ssrShader[0]->SetUniform("pixelStrideZCutoff", 50.f);
    ssrShader[0]->SetUniform("maxRayDistance", 500.f);
    ssrShader[0]->SetUniform("screenEdgeFadeStart", 0.9f);
    ssrShader[0]->SetUniform("eyeFadeStart", 0.2f);
    ssrShader[0]->SetUniform("eyeFadeEnd", 0.8f);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLCamera::ShowAmbientOcclusion(glm::vec4 rect)
{
	if(hasAO())
		((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, getAOTexture());
}

GLuint OpenGLCamera::getAOTexture()
{
    return aoBlurTex;
}

void OpenGLCamera::EnterPostprocessing()
{
	//Blit multisampled to non-multisampled texture
	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFBO);
	
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, viewportWidth, viewportHeight, 0, 0, viewportWidth, viewportHeight,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, viewportWidth, viewportHeight, 0, 0, viewportWidth, viewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

GLuint OpenGLCamera::getPostprocessTexture(unsigned int id)
{
	if(id < 2)
		return postprocessTex[id];
	else
		return 0;
}

void OpenGLCamera::DrawLDR(GLuint destinationFBO)
{
    //Bind HDR texture
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
    
    //Matrix light metering
    glBindFramebuffer(GL_FRAMEBUFFER, lightMeterFBO);
	glViewport(0,0,2,2);
    lightMeterShader->Use();
    lightMeterShader->SetUniform("texHDR", TEX_POSTPROCESS1);
    lightMeterShader->SetUniform("samples", glm::ivec2(64,64));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //Bind exposure texture
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, lightMeterTex);
    
    //LDR drawing
    glBindFramebuffer(GL_FRAMEBUFFER, destinationFBO);
	glViewport(0,0,viewportWidth,viewportHeight);
    tonemapShader->Use();
    tonemapShader->SetUniform("texHDR", TEX_POSTPROCESS1);
    tonemapShader->SetUniform("texAverage", TEX_POSTPROCESS2);
    tonemapShader->SetUniform("exposureComp", (GLfloat)powf(2.f,exposureComp));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
	glUseProgram(0); //Disable shaders
    
    //Unbind textures
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////// Static /////////////////////////////
void OpenGLCamera::Init()
{
    /////Tonemapping//////
	depthAwareBlurShader = new GLSLShader*[2];
	depthAwareBlurShader[0] = new GLSLShader("depthAwareBlur.frag", "gaussianBlur.vert");
	depthAwareBlurShader[0]->AddUniform("texSource", ParameterType::INT);
	depthAwareBlurShader[0]->AddUniform("texLinearDepth", ParameterType::INT);
	depthAwareBlurShader[0]->AddUniform("texelOffset", ParameterType::VEC2);
		
	depthAwareBlurShader[1] = new GLSLShader("depthAwareBlur2.frag", "gaussianBlur.vert");
	depthAwareBlurShader[1]->AddUniform("texSource", ParameterType::INT);
	depthAwareBlurShader[1]->AddUniform("texLinearDepth", ParameterType::INT);
	depthAwareBlurShader[1]->AddUniform("texelOffset", ParameterType::VEC2);
	depthAwareBlurShader[1]->AddUniform("sourceLayer", ParameterType::INT);
		
    lightMeterShader = new GLSLShader("lightMeter.frag");
    lightMeterShader->AddUniform("texHDR", ParameterType::INT);
    lightMeterShader->AddUniform("samples", ParameterType::IVEC2);
    
    tonemapShader = new GLSLShader("tonemapping.frag");
    tonemapShader->AddUniform("texHDR", ParameterType::INT);
    tonemapShader->AddUniform("texAverage", ParameterType::INT);
    tonemapShader->AddUniform("exposureComp", ParameterType::FLOAT);
	
    /////Linear depth////
    depthLinearizeShader = new GLSLShader*[2];
    depthLinearizeShader[0] = new GLSLShader("depthLinearize.frag");
    depthLinearizeShader[0]->AddUniform("clipInfo", ParameterType::VEC4);
    depthLinearizeShader[0]->AddUniform("texDepth", ParameterType::INT);
    depthLinearizeShader[1] = new GLSLShader("depthLinearizeMSAA.frag");
    depthLinearizeShader[1]->AddUniform("clipInfo", ParameterType::VEC4);
    depthLinearizeShader[1]->AddUniform("sampleIndex", ParameterType::INT);
    depthLinearizeShader[1]->AddUniform("texDepth", ParameterType::INT);
    
	/////AO//////////////
    if(GLEW_VERSION_4_3)
    {
        aoDeinterleaveShader = new GLSLShader("hbaoDeinterleave.frag");
        aoDeinterleaveShader->AddUniform("info", ParameterType::VEC4);
        aoDeinterleaveShader->AddUniform("texLinearDepth", ParameterType::INT);
        
        aoCalcShader = new GLSLShader*[2];
        aoCalcShader[0] = new GLSLShader("hbaoCalc.frag", "", "saq.geom");
        aoCalcShader[0]->AddUniform("texLinearDepth", ParameterType::INT);
        aoCalcShader[0]->AddUniform("texViewNormal", ParameterType::INT);
        aoCalcShader[1] = new GLSLShader("hbaoCalcMSAA.frag", "", "saq.geom");
        aoCalcShader[1]->AddUniform("texLinearDepth", ParameterType::INT);
        aoCalcShader[1]->AddUniform("texViewNormal", ParameterType::INT);
        aoCalcShader[1]->AddUniform("sampleIndex", ParameterType::INT);
        
        aoReinterleaveShader = new GLSLShader("hbaoReinterleave.frag");
        aoReinterleaveShader->AddUniform("texResultArray", ParameterType::INT);
        
        aoBlurShader = new GLSLShader*[2];
        aoBlurShader[0] = new GLSLShader("hbaoBlur.frag");
        aoBlurShader[0]->AddUniform("sharpness", ParameterType::FLOAT);
        aoBlurShader[0]->AddUniform("invResolutionDirection", ParameterType::VEC2);
        aoBlurShader[0]->AddUniform("texSource", ParameterType::INT);
        aoBlurShader[1] = new GLSLShader("hbaoBlur2.frag");
        aoBlurShader[1]->AddUniform("sharpness", ParameterType::FLOAT);
        aoBlurShader[1]->AddUniform("invResolutionDirection", ParameterType::VEC2);
        aoBlurShader[1]->AddUniform("texSource", ParameterType::INT);
    }
    
    //SSR - Screen Space Reflections
    ssrShader = new GLSLShader*[1];
    ssrShader[0] = new GLSLShader("ssr.frag");
    ssrShader[0]->AddUniform("texColor", ParameterType::INT);
    ssrShader[0]->AddUniform("texViewNormal", ParameterType::INT);
    ssrShader[0]->AddUniform("texLinearDepth", ParameterType::INT);
    ssrShader[0]->AddUniform("texLinearBackfaceDepth", ParameterType::INT);
    ssrShader[0]->AddUniform("P", ParameterType::MAT4);
    ssrShader[0]->AddUniform("invP", ParameterType::MAT4);
    ssrShader[0]->AddUniform("projInfo", ParameterType::VEC4);
    ssrShader[0]->AddUniform("viewportSize", ParameterType::VEC2);
    ssrShader[0]->AddUniform("invViewportSize", ParameterType::VEC2);
    ssrShader[0]->AddUniform("near", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("far", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("maxIterations", ParameterType::INT);
    ssrShader[0]->AddUniform("maxBinarySearchIterations", ParameterType::INT);
    ssrShader[0]->AddUniform("pixelZSize", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("pixelStride", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("pixelStrideZCutoff", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("maxRayDistance", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("screenEdgeFadeStart", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("eyeFadeStart", ParameterType::FLOAT);
    ssrShader[0]->AddUniform("eyeFadeEnd", ParameterType::FLOAT);
}

void OpenGLCamera::Destroy()
{
    if(lightMeterShader != NULL) delete lightMeterShader;
    if(tonemapShader != NULL) delete tonemapShader;
	
    if(depthLinearizeShader != NULL)
    {
        if(depthLinearizeShader[0] != NULL) delete depthLinearizeShader[0];
        if(depthLinearizeShader[1] != NULL) delete depthLinearizeShader[1];
    }
	
    if(aoDeinterleaveShader != NULL) delete aoDeinterleaveShader;
	
    if(aoCalcShader != NULL)
    {
        if(aoCalcShader[0] != NULL) delete aoCalcShader[0];
        if(aoCalcShader[1] != NULL) delete aoCalcShader[1];
    }
    
	if(aoReinterleaveShader != NULL) delete aoReinterleaveShader;
	
    if(aoBlurShader != NULL)
    {
        if(aoBlurShader[0] != NULL) delete aoBlurShader[0];
        if(aoBlurShader[1] != NULL) delete aoBlurShader[1];
    }
    
    if(ssrShader != NULL)
    {
        if(ssrShader[0] != NULL) delete ssrShader[0];
    }
}

}
