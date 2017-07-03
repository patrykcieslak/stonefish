//
//  OpenGLView.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLView__
#define __Stonefish_OpenGLView__

#include "OpenGLPipeline.h"
#include "GLSLShader.h"
#include "SolidEntity.h"

#define SCENE_ATTACHMENT        GL_COLOR_ATTACHMENT1
#define FINAL_ATTACHMENT        GL_COLOR_ATTACHMENT0

typedef struct
{
    GLfloat near;
    GLfloat far;
    GLfloat fov;
    GLfloat ratio;
    glm::vec3 corners[8];
}
ViewFrustum;

typedef enum {CAMERA, TRACKBALL} ViewType;
typedef enum {NORMAL, REFLECTED, REFRACTED} SceneComponent;

class OpenGLView
{
public:
    OpenGLView(GLint originX, GLint originY, GLint width, GLint height, GLfloat horizon, bool sao);
    virtual ~OpenGLView(void);
    
    virtual glm::mat4 GetViewTransform() = 0;
    virtual glm::vec3 GetEyePosition() = 0;
    virtual glm::vec3 GetLookingDirection() = 0;
    virtual glm::vec3 GetUpDirection() = 0;
    virtual ViewType getType() = 0;
    
    void SetupViewport(GLint x, GLint y, GLint width);
    void SetViewport();
    void SetProjection();
    void SetViewTransform();
    void ShowSceneTexture(SceneComponent sc, GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY);
    btVector3 Ray(GLint x, GLint y);

    void Activate();
    void Deactivate();
    void RenderSSAO();
    void RenderHDR(GLuint destinationFBO);
    void ShowAmbientOcclusion(GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY);
    
    GLint* GetViewport();
    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();
    GLfloat GetFOVY();
    GLfloat GetNearClip();
    GLfloat GetFarClip();
    
    GLuint getRenderFBO();
	GLuint getFinalTexture();
    GLuint getSSAOTexture();
    bool isActive();
    bool hasSSAO();
        
    static void Init();
    static void Destroy();
    static void SetTextureUnits(GLint position, GLint normal, GLint random);
    static GLuint getRandomTexture();
    
protected:
	//Multisampled float textures
    GLuint renderFBO;
    GLuint renderDepthStencil;
    GLuint renderColorTex;
	
	//Float texture
	GLuint lightMeterFBO;
    GLuint lightMeterTex;
	
	//RGB8 textures
	GLuint postprocessFBO;
	GLuint postprocessTex[2];
	int activePostprocessTexture;

    GLuint ssaoFBO;
    GLuint ssaoTexture;
    
    GLuint blurFBO;
    GLuint hBlurTexture;
    GLuint vBlurTexture;
	
    GLint originX;
    GLint originY;
    GLint viewportWidth;
    GLint viewportHeight;
    GLuint ssaoSizeDiv;
    GLfloat fovx;
    GLfloat near;
    GLfloat far;
    glm::mat4 projection;
    bool active;
    
    //downsample
    static GLSLShader* downsampleShader;
    
    //ssao
    static GLSLShader* ssaoShader;
    static GLSLShader* blurShader;
    static GLint positionTextureUnit;
    static GLint normalTextureUnit;
    static GLint randomTextureUnit;
    static GLuint randomTexture;
  
    //tonemapping
    static GLSLShader* lightMeterShader;
    static GLSLShader* tonemapShader;
};

#endif
