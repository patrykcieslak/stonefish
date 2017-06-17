//
//  OpenGLGBuffer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/18/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLGBuffer.h"
#include "OpenGLContent.h"
#include "Console.h"

void OpenGLGBuffer::SetClipPlane(double* plane)
{
    if(plane == NULL)
    {
        if(clipPlane != NULL)
        {
            delete [] clipPlane;
            clipPlane = NULL;
        }
    }
    else
    {
        if(clipPlane == NULL)
            clipPlane = new double[4];
        
        std::memcpy(clipPlane, plane, sizeof(double)*4);
        
        //glEnable(GL_CLIP_PLANE0);
        //glClipPlane(GL_CLIP_PLANE0, clipPlane);
    }
}

OpenGLGBuffer::OpenGLGBuffer(int fboWidth, int fboHeight)
{
    width  = fboWidth;
	height = fboHeight;
    rendering = false;
    clipPlane = NULL;
    
	// Generate the OGL resources for what we need
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &depthBuffer);
    
	// Bind the FBO so that the next operations will be bound to it
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
	// Bind the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
	// Generate and bind the OGL texture for diffuse
	glGenTextures(1, &diffuseTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuseTexture, 0);
    
	// Generate and bind the OGL texture for positions
	glGenTextures(2, positionTexture);
	glBindTexture(GL_TEXTURE_2D, positionTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); //32-bit precision needed for SAO
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positionTexture[0], 0);
    
    glBindTexture(GL_TEXTURE_2D, positionTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, positionTexture[1], 0);
    
	// Generate and bind the OGL texture for normals
	glGenTextures(2, normalsTexture);
	glBindTexture(GL_TEXTURE_2D, normalsTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normalsTexture[0], 0);
    
    glBindTexture(GL_TEXTURE_2D, normalsTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, normalsTexture[1], 0);
    
    // Check if all worked fine and unbind the FBO
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("GBuffer FBO initialization failed!");
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLGBuffer::~OpenGLGBuffer()
{
    glDeleteTextures(2, normalsTexture);
	glDeleteTextures(2, positionTexture);
	glDeleteTextures(1, &diffuseTexture);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &fbo);
}

void OpenGLGBuffer::Start(GLuint texIndex)
{
    rendering = true;
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	GLenum buffers[3];
    buffers[0] = GL_COLOR_ATTACHMENT0; //Diffuse reused
    
    if(texIndex == 0)
    {
        buffers[1] = GL_COLOR_ATTACHMENT1;
        buffers[2] = GL_COLOR_ATTACHMENT2;
    }
    else
    {
        buffers[1] = GL_COLOR_ATTACHMENT3;
        buffers[2] = GL_COLOR_ATTACHMENT4;
    }
    
    glDrawBuffers(3, buffers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLGBuffer::Stop()
{
    if(clipPlane != NULL)
        glDisable(GL_CLIP_PLANE0);
        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();
    
    rendering = false;
}

void OpenGLGBuffer::ShowTexture(FBOComponent component, GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY)
{
    GLuint texture;
    
    switch (component)
    {
        case DIFFUSE:
            texture = diffuseTexture;
            break;
            
        case POSITION1:
            texture = positionTexture[0];
            break;
            
        case POSITION2:
            texture = positionTexture[1];
            break;
            
        case NORMAL1:
            texture = normalsTexture[0];
            break;
            
        case NORMAL2:
            texture = normalsTexture[1];
            break;
    }
    
	OpenGLContent::getInstance()->DrawTexturedQuad(x, y, sizeX, sizeY, texture);
}