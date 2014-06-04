//
//  OpenGLGBuffer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/18/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLGBuffer.h"
#include "Console.h"

GLSLShader* OpenGLGBuffer::splittingShader = NULL;

void OpenGLGBuffer::LoadShaders()
{
    splittingShader = new GLSLShader("gbuffer.frag", "gbuffer.vert");
    splittingShader->AddUniform("isTextured", BOOLEAN);
    splittingShader->AddUniform("texture", INT);
    splittingShader->AddAttribute("materialData", FLOAT);
}

void OpenGLGBuffer::DeleteShaders()
{
    delete splittingShader;
}

void OpenGLGBuffer::SetAttributeMaterialData(GLfloat x)
{
    if(splittingShader->isEnabled())
        splittingShader->SetAttribute("materialData", x);
}

void OpenGLGBuffer::SetUniformIsTextured(bool x)
{
    if(splittingShader->isEnabled())
        splittingShader->SetUniform("isTextured", x);
}

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
        
        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, clipPlane);
    }
}

OpenGLGBuffer::OpenGLGBuffer(int fboWidth, int fboHeight)
{
    width  = fboWidth;
	height = fboHeight;
    rendering = false;
    clipPlane = NULL;
    
	// Generate the OGL resources for what we need
	glGenFramebuffersEXT(1, &fbo);
	glGenRenderbuffersEXT(1, &depthBuffer);
    
	// Bind the FBO so that the next operations will be bound to it
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    
	// Bind the depth buffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER_EXT, depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
	// Generate and bind the OGL texture for diffuse
	glGenTextures(1, &diffuseTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, diffuseTexture, 0);
    
	// Generate and bind the OGL texture for positions
	glGenTextures(2, positionTexture);
	glBindTexture(GL_TEXTURE_2D, positionTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL); //32-bit precision needed for SAO
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, positionTexture[0], 0);
    
    glBindTexture(GL_TEXTURE_2D, positionTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, positionTexture[1], 0);
    
	// Generate and bind the OGL texture for normals
	glGenTextures(2, normalsTexture);
	glBindTexture(GL_TEXTURE_2D, normalsTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, normalsTexture[0], 0);
    
    glBindTexture(GL_TEXTURE_2D, normalsTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_TEXTURE_2D, normalsTexture[1], 0);
    
    // Check if all worked fine and unbind the FBO
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        cError("GBuffer FBO initialization failed!");
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

OpenGLGBuffer::~OpenGLGBuffer()
{
    glDeleteRenderbuffersEXT(1, &depthBuffer);
    glDeleteTextures(2, normalsTexture);
	glDeleteTextures(2, positionTexture);
	glDeleteTextures(1, &diffuseTexture);
    glDeleteFramebuffersEXT(1, &fbo);
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
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
	GLenum buffers[3];
    buffers[0] = GL_COLOR_ATTACHMENT0_EXT; //Diffuse reused
    
    if(texIndex == 0)
    {
        buffers[1] = GL_COLOR_ATTACHMENT1_EXT;
        buffers[2] = GL_COLOR_ATTACHMENT2_EXT;
    }
    else
    {
        buffers[1] = GL_COLOR_ATTACHMENT3_EXT;
        buffers[2] = GL_COLOR_ATTACHMENT4_EXT;
    }
    
    glDrawBuffers(3, buffers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
    splittingShader->Enable();
    splittingShader->SetUniform("texture", 0);
}

void OpenGLGBuffer::Stop()
{
    splittingShader->Disable();
    
    if(clipPlane != NULL)
        glDisable(GL_CLIP_PLANE0);
        
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
    
	//Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glm::mat4 proj = glm::ortho(0.f, (GLfloat)width, 0.f, (GLfloat)height, -1.f, 1.f);
	glLoadMatrixf(glm::value_ptr(proj));
	
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
	glVertex3f(0.0f,(float)height, 0.0f);
	glTexCoord2f(0, 0);
	glVertex3f(0.0f, height-sizeY, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(sizeX, height-sizeY, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(sizeX, (float)height, 0.0f);
	glEnd();
    
	glBindTexture(GL_TEXTURE_2D, 0);
    
	//Reset to the matrices
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}