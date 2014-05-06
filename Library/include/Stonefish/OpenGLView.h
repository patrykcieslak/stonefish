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
#include "FluidEntity.h"

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

class OpenGLView
{
public:
    
    OpenGLView(GLint originX, GLint originY, GLint width, GLint height, GLuint ssaoSize);
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
    btTransform GetReflectedViewTransform(const btVector3& normal, const btVector3& position);
    btTransform GetRefractedViewTransform(const btVector3& normal, const btVector3& position, const Fluid* fluid);

    void Activate();
    void Deactivate();
    void RenderSSAO();
    void RenderFluid(FluidEntity* fluid);
    void ShowAmbientOcclusion();
    
    GLint* GetViewport();
    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();
    GLfloat GetFOVY();
    GLfloat GetNearClip();
    GLfloat GetFarClip();
    
    GLuint getSceneFBO();
    GLuint getSceneTexture();
    GLuint getSceneReflectionTexture();
    GLuint getSceneRefractionTexture();
    GLuint getSSAOTexture();
    bool isActive();
    OpenGLGBuffer* getGBuffer();
    
    static void Init();
    static void Destroy();
    static void SetTextureUnits(GLint position, GLint normal, GLint random);
    static GLuint getRandomTexture();
    
protected:
    OpenGLGBuffer* gBuffer;
    
    GLuint sceneFBO;
    GLuint sceneTexture;
    GLuint sceneReflectionTexture;
    GLuint sceneRefractionTexture;
    GLuint sceneDepthBuffer;
    
    GLuint ssaoFBO;
    GLuint ssaoTexture;
    
    GLuint hBlurFBO;
    GLuint hBlurTexture;
    GLuint vBlurFBO;
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
    
    //ssao
    static GLhandleARB ssaoShader;
    static GLhandleARB blurShader;
    static GLint positionTextureUnit;
    static GLint normalTextureUnit;
    static GLint randomTextureUnit;
    static GLint uniSaoRandom, uniSaoPosition, uniSaoNormal, uniSaoRadius, uniSaoProjScale, uniSaoBias, uniSaoIntDivR6, uniSaoViewport;
    static GLint uniSaoBlurSource, uniSaoBlurAxis;
    static GLuint randomTexture;
    
    //fluid
    static GLhandleARB fluidShader[4];
    static GLint uniFluidReflection[2], uniFluidScene[4], uniFluidPosition[3], uniFluidViewport[3], uniFluidIP[2], uniFluidR0[2];
    static GLint uniFluidEyeSurfaceP[3], uniFluidEyeSurfaceN[3], uniFluidVisibility[3];
};

#endif
