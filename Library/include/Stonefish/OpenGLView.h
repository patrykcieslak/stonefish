//
//  OpenGLView.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLView__
#define __Stonefish_OpenGLView__

#include "OpenGLPipeline.h"
#include "OpenGLGBuffer.h"
#include "GLSLShader.h"
#include "FluidEntity.h"
#include "SolidEntity.h"

#define SCENE_ATTACHMENT        GL_COLOR_ATTACHMENT1_EXT
#define REFLECTION_ATTACHMENT   GL_COLOR_ATTACHMENT2_EXT
#define REFRACTION_ATTACHMENT   GL_COLOR_ATTACHMENT3_EXT
#define FINAL_ATTACHMENT        GL_COLOR_ATTACHMENT0_EXT

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
    
    virtual btTransform GetViewTransform() = 0;
    virtual btVector3 GetEyePosition() = 0;
    virtual btVector3 GetLookingDirection() = 0;
    virtual btVector3 GetUpDirection() = 0;
    virtual ViewType getType() = 0;
    
    void SetupViewport(GLint x, GLint y, GLint width);
    void SetViewport();
    void SetProjection();
    void SetViewTransform();
    void SetReflectedViewTransform(FluidEntity* fluid);
    void SetRefractedViewTransform(FluidEntity* fluid);
    btTransform GetReflectedViewTransform(const FluidEntity* fluid);
    btTransform GetRefractedViewTransform(const FluidEntity* fluid);
    void ShowSceneTexture(SceneComponent sc, GLfloat x, GLfloat y, GLfloat sizeX, GLfloat sizeY);
    btVector3 Ray(GLint x, GLint y);

    void Activate();
    void Deactivate();
    void RenderSSAO();
    void RenderFluidSurface(FluidEntity* fluid, bool underwater);
    void RenderFluidVolume(FluidEntity* fluid);
    void RenderHDR(GLuint destinationFBO);
    void ShowAmbientOcclusion();
    
    GLint* GetViewport();
    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix(const btTransform& viewTransform);
    GLfloat GetFOVY();
    GLfloat GetNearClip();
    GLfloat GetFarClip();
    
    GLuint getSceneFBO();
    GLuint getFinalTexture();
    GLuint getSceneTexture();
    GLuint getSceneReflectionTexture();
    GLuint getSceneRefractionTexture();
    GLuint getSSAOTexture();
    bool isActive();
    bool hasSSAO();
    OpenGLGBuffer* getGBuffer();
    
    static void Init();
    static void Destroy();
    static void SetTextureUnits(GLint position, GLint normal, GLint random);
    static GLuint getRandomTexture();
    
protected:
    OpenGLGBuffer* gBuffer;
    
    GLuint sceneFBO;
    GLuint finalTexture;
    GLuint sceneTexture;
    GLuint sceneReflectionTexture;
    GLuint sceneRefractionTexture;
    GLuint sceneDepthBuffer;
    
    GLuint ssaoFBO;
    GLuint ssaoTexture;
    
    GLuint blurFBO;
    GLuint hBlurTexture;
    GLuint vBlurTexture;
    
    GLuint lightMeterFBO;
    GLuint lightMeterTexture;
    
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
    
    //fluid
    static GLSLShader* fluidShader[4];
    static GLuint waveNormalTexture;
    
    //tonemapping
    static GLSLShader* lightMeterShader;
    static GLSLShader* tonemapShader;
};

#endif
