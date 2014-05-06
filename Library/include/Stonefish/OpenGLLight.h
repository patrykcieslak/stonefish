//
//  OpenGLLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLLight__
#define __Stonefish_OpenGLLight__

#include "OpenGLPipeline.h"
#include "Entity.h"
#include "OpenGLCamera.h"

#define Max(a, b)   (((a) > (b)) ? (a) : (b))

typedef struct
{
    GLfloat xRed, yRed,	    	    /* Red x, y */
    xGreen, yGreen,  	    /* Green x, y */
    xBlue, yBlue,    	    /* Blue x, y */
    xWhite, yWhite,  	    /* White point x, y */
    gamma;   	    	    /* Gamma correction for system */
}
ColorSystem;

class OpenGLLight
{
public:
    //Discrete lights
    OpenGLLight(const btVector3& position, GLfloat* color4);
    virtual ~OpenGLLight();
    
    virtual void Render() = 0;
    virtual void UpdateLight() = 0;
    virtual void RenderLightSurface() = 0;
    virtual void RenderDummy() = 0;
    virtual void RenderShadowMap(OpenGLPipeline* pipe) = 0;
    virtual void ShowShadowMap(GLfloat x, GLfloat y, GLfloat scale) = 0;
    void GlueToEntity(Entity* ent);
    void Activate();
    void Deactivate();
    bool isActive();
    void setLightSurfaceDistance(GLfloat dist);
    GLfloat* getColor();
    btVector3 getViewPosition();
    btVector3 getPosition();
    //btVector3 getPosition();
    Entity* getHoldingEntity();
    
    //Ambient light and shaders
    static void Init();
    static void Destroy();
    static void SetTextureUnits(GLint diffuse, GLint normal, GLint position, GLint skyDiffuse, GLint skyReflection, GLint ssao, GLint shadow);
    static void SetCamera(OpenGLView* view);
    static void RenderAmbientLight(const btTransform& viewTransform, bool zAxisUp);
    
    //Utilities
    static GLfloat* ColorFromTemperature(GLfloat temperatureK, GLfloat intensity);
    
protected:
    Entity* holdingEntity;
    bool active;
    btVector3 relpos;
    btVector3 pos;
    GLfloat surfaceDistance;
    GLfloat* color;
    
    static OpenGLView* activeView;
    static GLhandleARB omniShader;
    static GLhandleARB spotShader;
    
    static GLint diffuseTextureUnit, normalTextureUnit, positionTextureUnit, skyDiffuseTextureUnit, skyReflectionTextureUnit, ssaoTextureUnit, shadowTextureUnit;
    static GLint uniODiffuse, uniONormal, uniOPosition, uniOLightPos, uniOColor;
    static GLint uniSDiffuse, uniSNormal, uniSPosition, uniSLightPos, uniSLightDir, uniSLightAngle, uniSColor, uniSLightClipSpace, uniSShadow;
    
    static GLhandleARB ambientShader;
    static GLint uniADiffuse, uniANormal, uniAPosition, uniASkyDiff, uniASkyReflect, uniAIVR, uniAIP, uniAViewport, uniASsao;
    
    static ColorSystem cs;
    static GLfloat bbSpectrum(GLfloat wavelength, GLfloat temperature);
    static void bbSpectrumToXYZ(GLfloat temperature, GLfloat& x, GLfloat& y, GLfloat& z);
    static void xyzToRGB(GLfloat x, GLfloat y, GLfloat z, GLfloat& r, GLfloat& g, GLfloat& b);
};

#endif
