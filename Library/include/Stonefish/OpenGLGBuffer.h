//
//  OpenGLGBuffer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/18/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLGBuffer__
#define __Stonefish_OpenGLGBuffer__

#include "OpenGLPipeline.h"

typedef enum {DIFFUSE, POSITION1, POSITION2, NORMAL1, NORMAL2} FBOComponent;

class OpenGLGBuffer
{
public:
    OpenGLGBuffer(int fboWidth, int fboHeight);
    ~OpenGLGBuffer();
    
    void Start(GLuint texIndex);
	void Stop();
    void ShowTexture(FBOComponent component, GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY);
    void SetClipPlane(GLfloat* planeEq);
    
	GLuint getDiffuseTexture() const { return diffuseTexture; }
	GLuint getPositionTexture(GLuint index) const { return positionTexture[index]; }
	GLuint getNormalsTexture(GLuint index) const { return normalsTexture[index]; }
    
    static void LoadShaders();
    static void DeleteShaders();
    static GLint getUniformLocation_IsTextured();
    static GLint getAttributeLocation_MaterialData();
    
private:
    GLuint fbo;                // FBO handle
	GLuint diffuseTexture;     // texture for the diffuse render target
	GLuint positionTexture[2]; // texture for the position render target
	GLuint normalsTexture[2];  // texture for the normals render target
	GLuint depthBuffer;        // depth buffer handle
    int	width;  // FBO width
	int	height; // FBO height
    bool rendering;
    
    static GLhandleARB splittingShader;
    static GLint uniIsTextured, uniTexture, attMatData, uniClipPlane;
};

#endif