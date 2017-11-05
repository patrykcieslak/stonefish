//
//  OpenGLView.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLView.h"
#include "MathsUtil.hpp"
#include "SimulationApp.h"
#include "Console.h"
#include "SystemUtil.hpp"
#include "MersenneTwister.hpp"

GLSLShader** OpenGLView::depthAwareBlurShader = NULL;
GLSLShader* OpenGLView::lightMeterShader = NULL;
GLSLShader* OpenGLView::tonemapShader = NULL;
GLSLShader** OpenGLView::depthLinearizeShader = NULL;
GLSLShader* OpenGLView::aoDeinterleaveShader = NULL;
GLSLShader** OpenGLView::aoCalcShader = NULL;
GLSLShader* OpenGLView::aoReinterleaveShader = NULL;
GLSLShader** OpenGLView::aoBlurShader = NULL;

OpenGLView::OpenGLView(GLint x, GLint y, GLint width, GLint height, GLfloat horizon, GLuint spp, bool ao)
{
    viewportWidth = width;
    viewportHeight = height;
    originX = x;
    originY = y;
    fovx = 0.785f;
    active = false;
    aoFactor = ao ? 1 : 0;
	samples = spp < 1 ? 1 : (spp > 8 ? 8 : spp);
    far = UnitSystem::SetLength(horizon);
    near = 0.1f;
	activePostprocessTexture = 0;
    
	//----Geometry rendering----
	//Normal render buffer
	if(samples > 1) //MSAA
	{
		glGenTextures(1, &renderColorTex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderColorTex);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB16F, viewportWidth, viewportHeight, GL_FALSE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		
		glGenTextures(1, &renderViewNormalTex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderViewNormalTex);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, viewportWidth, viewportHeight, GL_FALSE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		
		glGenTextures(1, &renderDepthStencilTex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderDepthStencilTex);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight, GL_FALSE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}
	else
	{
		glGenTextures(1, &renderColorTex);
		glBindTexture(GL_TEXTURE_2D, renderColorTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, viewportWidth, viewportHeight);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glGenTextures(1, &renderViewNormalTex);
		glBindTexture(GL_TEXTURE_2D, renderViewNormalTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, viewportWidth, viewportHeight);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glGenTextures(1, &renderDepthStencilTex);
		glBindTexture(GL_TEXTURE_2D, renderDepthStencilTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight);
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
		
	//Planar reflection render buffer (half-size, no multisampling)
	glGenTextures(1, &reflectionColorTex);
	glBindTexture(GL_TEXTURE_2D, reflectionColorTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, viewportWidth, viewportHeight);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenTextures(1, &reflectionDepthStencilTex);
	glBindTexture(GL_TEXTURE_2D, reflectionDepthStencilTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, viewportWidth, viewportHeight);
	glBindTexture(GL_TEXTURE_2D, 0);
		
	glGenFramebuffers(1, &reflectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflectionColorTex, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, reflectionDepthStencilTex, 0);
		
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Planar reflection FBO initialization failed!");
    
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
	
	glGenTextures(3, postprocessTex);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewportWidth, viewportHeight, 0, GL_RGB, GL_FLOAT, NULL);
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
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, postprocessTex[2]);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB16F, viewportWidth, viewportHeight, 5);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Postprocess FBO initialization failed!");
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//Linear depth
	glGenTextures(1, &linearDepthTex);
	glBindTexture(GL_TEXTURE_2D, linearDepthTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, viewportWidth, viewportHeight);
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
		MTRand rng;
		float numDir = 8; // keep in sync to glsl
		rng.seed((unsigned)0);

		for(int i=0; i<HBAO_RANDOM_ELEMENTS; ++i)
		{
			float Rand1 = rng.randExc();
			float Rand2 = rng.randExc();

			//Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
			float angle = 2.f * M_PI * Rand1 / numDir;
			aoData.jitters[i].x = cosf(angle);
			aoData.jitters[i].y = sinf(angle);
			aoData.jitters[i].z = Rand2;
			aoData.jitters[i].w = 0;
			
			aoData.float2Offsets[i] = glm::vec4((GLfloat)(i%4) + 0.5f, (GLfloat)(i/4) + 0.5f, 0.0, 0.0);
		}
    }
}

OpenGLView::~OpenGLView()
{
    glDeleteTextures(3, postprocessTex);
	glDeleteTextures(1, &renderColorTex);
	glDeleteTextures(1, &renderViewNormalTex);
	glDeleteTextures(1, &renderDepthStencilTex);
	glDeleteTextures(1, &reflectionColorTex);
	glDeleteTextures(1, &reflectionDepthStencilTex);
    glDeleteTextures(1, &lightMeterTex);
	glDeleteTextures(1, &linearDepthTex);
	
	glDeleteFramebuffers(1, &renderFBO);
	glDeleteFramebuffers(1, &reflectionFBO);
    glDeleteFramebuffers(1, &postprocessFBO);
	glDeleteFramebuffers(1, &lightMeterFBO);
    
    if(aoFactor > 0)
    {
		glDeleteTextures(1, &aoResultTex);
		glDeleteTextures(1, &aoBlurTex);
		glDeleteTextures(1, &aoDepthArrayTex);
		glDeleteTextures(1, &aoResultArrayTex);
		glDeleteTextures(HBAO_RANDOM_ELEMENTS, aoDepthViewTex);
	
		glDeleteFramebuffers(1, &linearDepthFBO);
		glDeleteFramebuffers(1, &aoFinalFBO);
		glDeleteFramebuffers(1, &aoDeinterleaveFBO);
		glDeleteFramebuffers(1, &aoCalcFBO);
		
		glDeleteBuffers(1, &aoDataUBO);
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

GLint* OpenGLView::GetViewport() const
{
	GLint* view = new GLint[4];
	view[0] = originX;
	view[1] = originY;
	view[2] = viewportWidth;
	view[3] = viewportHeight;
    return view;
}

glm::mat4 OpenGLView::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLView::GetInfiniteProjectionMatrix() const
{
	GLfloat aspect = (GLfloat)viewportWidth/(GLfloat)viewportHeight;
    GLfloat fovy = fovx/aspect;
	GLfloat tanHalfFovy = tan(fovy/2.f);
	
	glm::mat4 infProj(0);
	infProj[0][0] = 1.f/(aspect * tanHalfFovy);
	infProj[1][1] = 1.f/tanHalfFovy;
	infProj[2][2] = -1.f;
	infProj[2][3] = -1.f;
	infProj[3][2] = -2.f*near;
	return infProj;
}

glm::mat4 OpenGLView::GetViewMatrix() const
{
	return GetViewTransform();
}

GLfloat OpenGLView::GetFOVX() const
{
	return fovx;
}

GLfloat OpenGLView::GetFOVY() const
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

GLuint OpenGLView::getReflectionFBO()
{
	return reflectionFBO;
}

GLuint OpenGLView::getFinalTexture()
{
    return postprocessTex[activePostprocessTexture];
}

GLuint OpenGLView::getReflectionTexture()
{
	return reflectionColorTex;
}

bool OpenGLView::hasAO()
{
    return aoFactor > 0;
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

void OpenGLView::SetReflectionViewport()
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

void OpenGLView::ShowSceneTexture(SceneComponent sc, glm::vec4 rect)
{
    GLuint texture;
    
    switch (sc)
    {
		default:
        case NORMAL:
            texture = renderColorTex;
            break;
			
		case REFLECTED:
			texture = reflectionColorTex;
			break;
    }
    
    OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, texture);
}

void OpenGLView::ShowLinearDepthTexture(glm::vec4 rect)
{
	if(hasAO())
		OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, linearDepthTex);
}

void OpenGLView::ShowViewNormalTexture(glm::vec4 rect)
{
	if(samples>1)
		OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex, glm::ivec2(viewportWidth, viewportHeight));
	else
		OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, renderViewNormalTex);
}

void OpenGLView::ShowDeinterleavedDepthTexture(glm::vec4 rect, GLuint index)
{
	if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
		OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoDepthArrayTex, index);
}

void OpenGLView::ShowDeinterleavedAOTexture(glm::vec4 rect, GLuint index)
{
	if(hasAO() && index < HBAO_RANDOM_ELEMENTS)
		OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, aoResultArrayTex, index);
}

void OpenGLView::GenerateBlurArray()
{
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
	glViewport(0,0,viewportWidth,viewportHeight);
	OpenGLContent::getInstance()->BindBaseVertexArray();
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, getLinearDepthTexture()); //Always attached
	
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, postprocessTex[2], 0, 0); //First layer
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, postprocessTex[0]); //Initial texture to use as source
	
	depthAwareBlurShader[0]->Use();
	depthAwareBlurShader[0]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
	depthAwareBlurShader[0]->SetUniform("texSource", TEX_POSTPROCESS2);
	depthAwareBlurShader[0]->SetUniform("texelOffset", glm::vec2(1.f/(GLfloat)viewportWidth, 0.f)); //Horizontal blur
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, postprocessTex[2]); //Attach blur array
	
	depthAwareBlurShader[1]->Use();
	depthAwareBlurShader[1]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
	depthAwareBlurShader[1]->SetUniform("texSource", TEX_POSTPROCESS2);
	
	for(int i=1; i<8; ++i)
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, postprocessTex[2], 0, i%2 == 0 ? 0 : i/2+1); //First layer
		depthAwareBlurShader[1]->SetUniform("texelOffset", i%2 == 0 ? glm::vec2(1.f/(GLfloat)viewportWidth, 0.f) : glm::vec2(0.f, ((GLfloat)viewportHeight/(GLfloat)viewportWidth)/(GLfloat)viewportWidth));
		depthAwareBlurShader[1]->SetUniform("sourceLayer", i%2 == 0 ? i/2 : 0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
    
	glUseProgram(0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, 0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLView::GenerateLinearDepth(int sampleId)
{
	glBindFramebuffer(GL_FRAMEBUFFER, linearDepthFBO);
	glViewport(0, 0, viewportWidth, viewportHeight);
	OpenGLContent::getInstance()->BindBaseVertexArray();
	
	if(samples>1)
	{
		depthLinearizeShader[1]->Use();
		depthLinearizeShader[1]->SetUniform("clipInfo", glm::vec4(near*far, near-far, far, 1.f));
		depthLinearizeShader[1]->SetUniform("sampleIndex", sampleId);
		depthLinearizeShader[1]->SetUniform("texDepth", TEX_POSTPROCESS1);
			
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_MULTISAMPLE, renderDepthStencilTex);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_MULTISAMPLE, 0);
	}
	else
	{
		depthLinearizeShader[0]->Use();
		depthLinearizeShader[0]->SetUniform("clipInfo", glm::vec4(near*far, near-far, far, 1.f));
		depthLinearizeShader[0]->SetUniform("texDepth", TEX_POSTPROCESS1);
		
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, renderDepthStencilTex);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
	}
	
	glUseProgram(0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint OpenGLView::getLinearDepthTexture()
{
	return linearDepthTex;
}

void OpenGLView::DrawAO()
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
		aoData.PowExponent = 1.5f; //intensity
		aoData.NDotVBias = 0.01f;  //<0,1>
		aoData.AOMultiplier = 1.f/(1.f-aoData.NDotVBias);
		aoData.InvQuarterResolution = invQuarterRes;
		aoData.InvFullResolution = invFullRes;
		
		GLfloat blurSharpness = 40.0f;
		
		//For all samples
		for(int n=0; n<samples; ++n)
		{
			GenerateLinearDepth(n);
			OpenGLContent::getInstance()->BindBaseVertexArray(); //Previous function unbinds vertex array
		
			//Deinterleave
			glBindFramebuffer(GL_FRAMEBUFFER, aoDeinterleaveFBO);
			glViewport(0, 0, quarterWidth, quarterHeight);

			aoDeinterleaveShader->Use();
			aoDeinterleaveShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
		
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex);
			for(int i=0; i<HBAO_RANDOM_ELEMENTS; i+=NUM_MRT)
			{
				aoDeinterleaveShader->SetUniform("info", glm::vec4(float(i % 4) + 0.5f, float(i / 4) + 0.5f, invFullRes.x, invFullRes.y));
			
				for(int layer = 0; layer < NUM_MRT; ++layer)
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, aoDepthViewTex[i+layer], 0);
			
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
			
			//Calculate HBAO
			glBindFramebuffer(GL_FRAMEBUFFER, aoCalcFBO);
			glViewport(0, 0, quarterWidth, quarterHeight);
			
			if(samples>1)
			{
				aoCalcShader[1]->Use();
				aoCalcShader[1]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
				aoCalcShader[1]->SetUniform("texViewNormal", TEX_POSTPROCESS2);
				aoCalcShader[1]->SetUniform("sampleIndex", n);
		
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_MULTISAMPLE, renderViewNormalTex);
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, aoDataUBO);
				glNamedBufferSubDataEXT(aoDataUBO, 0, sizeof(AOData), &aoData);
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
		
				glDrawArrays(GL_TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
				
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_MULTISAMPLE, 0);
			}
			else
			{
				aoCalcShader[0]->Use();
				aoCalcShader[0]->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
				aoCalcShader[0]->SetUniform("texViewNormal", TEX_POSTPROCESS2);
		
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, renderViewNormalTex);
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, aoDataUBO);
				glNamedBufferSubDataEXT(aoDataUBO, 0, sizeof(AOData), &aoData);
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoDepthArrayTex);
		
				glDrawArrays(GL_TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
				
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
				glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
			}
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, viewportWidth, viewportHeight);
		
			//Reinterleave
			glBindFramebuffer(GL_FRAMEBUFFER, aoFinalFBO);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glViewport(0, 0, viewportWidth, viewportHeight);
			aoReinterleaveShader->Use();
			aoReinterleaveShader->SetUniform("texResultArray", TEX_POSTPROCESS1);
		
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, aoResultArrayTex);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
		
			//Blur
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			aoBlurShader[0]->Use();
			aoBlurShader[0]->SetUniform("sharpness", blurSharpness);
			aoBlurShader[0]->SetUniform("invResolutionDirection", glm::vec2(1.f/(GLfloat)viewportWidth, 0));
			aoBlurShader[0]->SetUniform("texSource", TEX_POSTPROCESS1);
		
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, aoResultTex);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);

			//Final output to main fbo
			glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glDepthMask(GL_FALSE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
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
		
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, aoBlurTex);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
		
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

void OpenGLView::ShowAmbientOcclusion(glm::vec4 rect)
{
	if(hasAO())
		OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, getAOTexture());
}

GLuint OpenGLView::getAOTexture()
{
    return aoBlurTex;
}

void OpenGLView::EnterPostprocessing()
{
	//Blit multisampled to non-multisampled texture
	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFBO);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, postprocessTex[0], 0);
	glBlitFramebuffer(0, 0, viewportWidth, viewportHeight, 0, 0, viewportWidth, viewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

GLuint OpenGLView::getPostprocessTexture(unsigned int id)
{
	if(id < 3)
		return postprocessTex[id];
	else
		return 0;
}

void OpenGLView::DrawHDR(GLuint destinationFBO)
{
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, postprocessTex[0]);
	
    //matrix light metering
    glBindFramebuffer(GL_FRAMEBUFFER, lightMeterFBO);
    lightMeterShader->Use();
    lightMeterShader->SetUniform("texHDR", TEX_POSTPROCESS1);
    lightMeterShader->SetUniform("samples", glm::ivec2(128,128));
    OpenGLContent::getInstance()->DrawSAQ();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //hdr drawing
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lightMeterTex);
    
    glBindFramebuffer(GL_FRAMEBUFFER, destinationFBO);
    tonemapShader->Use();
    tonemapShader->SetUniform("texHDR", TEX_POSTPROCESS1);
    tonemapShader->SetUniform("texAverage", TEX_POSTPROCESS2);
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
	
	/////AO//////////////
	depthLinearizeShader = new GLSLShader*[2];
	depthLinearizeShader[0] = new GLSLShader("depthLinearize.frag");
	depthLinearizeShader[0]->AddUniform("clipInfo", ParameterType::VEC4);
	depthLinearizeShader[0]->AddUniform("texDepth", ParameterType::INT);
	depthLinearizeShader[1] = new GLSLShader("depthLinearizeMSAA.frag");
	depthLinearizeShader[1]->AddUniform("clipInfo", ParameterType::VEC4);
	depthLinearizeShader[1]->AddUniform("sampleIndex", ParameterType::INT);
	depthLinearizeShader[1]->AddUniform("texDepth", ParameterType::INT);
		
	aoDeinterleaveShader = new GLSLShader("hbaoDeinterleave.frag");
	aoDeinterleaveShader->AddUniform("info", ParameterType::VEC4);
	aoDeinterleaveShader->AddUniform("texLinearDepth", ParameterType::INT);
	
	aoCalcShader = new GLSLShader*[2];
	aoCalcShader[0] = new GLSLShader("hbaoCalc.frag", "", "hbaoSaq.geom");
	aoCalcShader[0]->AddUniform("texLinearDepth", ParameterType::INT);
	aoCalcShader[0]->AddUniform("texViewNormal", ParameterType::INT);
	aoCalcShader[1] = new GLSLShader("hbaoCalcMSAA.frag", "", "hbaoSaq.geom");
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

void OpenGLView::Destroy()
{
    if(lightMeterShader != NULL) delete lightMeterShader;
    if(tonemapShader != NULL) delete tonemapShader;
	if(depthLinearizeShader[0] != NULL) delete depthLinearizeShader[0];
	if(depthLinearizeShader[1] != NULL) delete depthLinearizeShader[1];
	if(aoDeinterleaveShader != NULL) delete aoDeinterleaveShader;
	if(aoCalcShader[0] != NULL) delete aoCalcShader[0];
	if(aoCalcShader[1] != NULL) delete aoCalcShader[1];
	if(aoReinterleaveShader != NULL) delete aoReinterleaveShader;
	if(aoBlurShader[0] != NULL) delete aoBlurShader[0];
	if(aoBlurShader[1] != NULL) delete aoBlurShader[1];
}