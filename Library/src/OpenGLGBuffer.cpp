//
//  OpenGLGBuffer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/18/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "OpenGLGBuffer.h"

#include <stdio.h>
#include "OpenGLUtil.h"

GLhandleARB OpenGLGBuffer::splittingShader = NULL;
GLint OpenGLGBuffer::uniIsTextured = 0;
GLint OpenGLGBuffer::uniTexture = 0;
GLint OpenGLGBuffer::attMatData = 0;
GLint OpenGLGBuffer::uniClipPlane = 0;

void OpenGLGBuffer::LoadShaders()
{
    GLhandleARB vertexShader, fragmentShader;
    GLint compiled;
    vertexShader = LoadShader(GL_VERTEX_SHADER, "gbuffer.vert", &compiled);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, "gbuffer.frag", &compiled);
    splittingShader = CreateProgramObject(vertexShader, fragmentShader);
    LinkProgram(splittingShader, &compiled);
    
    glUseProgramObjectARB(splittingShader);
    uniIsTextured = glGetUniformLocationARB(splittingShader, "isTextured");
    uniTexture = glGetUniformLocationARB(splittingShader, "texture");
    attMatData = glGetAttribLocationARB(splittingShader, "materialData");
    uniClipPlane = glGetUniformLocationARB(splittingShader, "clipPlane");
    glUseProgramObjectARB(0);
}

void OpenGLGBuffer::DeleteShaders()
{
    glDeleteObjectARB(splittingShader);
}

GLint OpenGLGBuffer::getAttributeLocation_MaterialData()
{
    return attMatData;
}

GLint OpenGLGBuffer::getUniformLocation_IsTextured()
{
    return uniIsTextured;
}

void OpenGLGBuffer::SetClipPlane(GLfloat *planeEq)
{
    if(planeEq == NULL)
    {
        GLfloat eq[4];
        eq[0] = eq[1] = eq[2] = eq[3] = 0.f;
        glUniform4fvARB(uniClipPlane, 1, eq);
    }
    else
        glUniform4fvARB(uniClipPlane, 1, planeEq);
}

OpenGLGBuffer::OpenGLGBuffer(int fboWidth, int fboHeight)
{
    width  = fboWidth;
	height = fboHeight;
    rendering = false;
    
	// Generate the OGL resources for what we need
	glGenFramebuffersEXT(1, &fbo);
	glGenRenderbuffersEXT(1, &depthBuffer);
    
	// Bind the FBO so that the next operations will be bound to it
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    
	// Bind the depth buffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
	// Generate and bind the OGL texture for diffuse
	glGenTextures(1, &diffuseTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, diffuseTexture, 0);
    
	// Generate and bind the OGL texture for positions
	glGenTextures(2, positionTexture);
	glBindTexture(GL_TEXTURE_2D, positionTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, positionTexture[0], 0);
    
    glBindTexture(GL_TEXTURE_2D, positionTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, positionTexture[1], 0);
    
	// Generate and bind the OGL texture for normals
	glGenTextures(2, normalsTexture);
	glBindTexture(GL_TEXTURE_2D, normalsTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, normalsTexture[0], 0);
    
    glBindTexture(GL_TEXTURE_2D, normalsTexture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Attach the texture to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_TEXTURE_2D, normalsTexture[1], 0);
    
    // Check if all worked fine and unbind the FBO
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        printf("FBO initialization failed.\n");
	
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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
	GLenum buffers[3];
    buffers[0] = GL_COLOR_ATTACHMENT0_EXT;
    
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
	
    glUseProgramObjectARB(splittingShader);
    glUniform1iARB(uniTexture, 0);
}

void OpenGLGBuffer::Stop()
{
    glUseProgramObjectARB(0);
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
	glLoadIdentity();
	glOrtho(0,width,0,height,0.1f,2.f);
    
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