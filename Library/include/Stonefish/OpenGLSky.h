//
//  OpenGLSky.h
//  Stonefish
//
//  Created by Patryk Cieslak on 9/18/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSky__
#define __Stonefish_OpenGLSky__

#include "OpenGLPipeline.h"
#include "GLSLShader.h"
#include "OpenGLView.h"

typedef enum {SKY,DOWNSAMPLE2, DOWNSAMPLE4, DOWNSAMPLE8, CONVOLUTION_DIFFUSE, CONVOLUTION_REFLECT} SkyCubemap;

//singleton
class OpenGLSky
{
public:
    void Init();
    void Generate(GLfloat elevation, GLfloat azimuth);
    void Render(OpenGLView* view, bool zAxisUp);
    GLuint getSkyCubemap();
    GLuint getDiffuseCubemap();
    GLuint getReflectionCubemap();
    void ShowCubemap(SkyCubemap cmap);
    
    static OpenGLSky* getInstance();
 
private:
    OpenGLSky();
    ~OpenGLSky();
    void ProcessCube(GLSLShader* shader, GLuint cubemap, GLenum attachment);
    
    //Sky params
    GLfloat sunElevation;
    GLfloat sunAzimuth;
    
    //Sky cubemap rendering
    //framebuffer
    GLuint skyCubeFBO;
    GLsizei skyCubeSize;
    GLuint skyCubemap;
    //shader
    GLSLShader* skyCubeShader;
    
    //Downsampling
    //framebuffer
    GLuint ds2FBO;
    GLuint ds4FBO;
    GLuint ds8FBO;
    GLuint ds2Cubemap;
    GLuint ds4Cubemap;
    GLuint ds8Cubemap;
    //shader
    GLSLShader* dsShader;
    
    //Convolution
    //framebuffer
    GLuint convolveFBO;
    GLuint convolveDiffuseCubemap;
    GLuint convolveReflectionCubemap;
    //shader
    GLSLShader* convolveShader;
    
    //Sky drawing
    //shader
    GLSLShader* skyDrawShader;
    
    //misc
    GLfloat debugAngle;
    
    static OpenGLSky* instance;
};


#endif