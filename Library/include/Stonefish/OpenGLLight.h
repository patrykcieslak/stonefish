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
#include "GLSLShader.h"
#include "SolidEntity.h"
#include "OpenGLCamera.h"

#define Max(a, b)   (((a) > (b)) ? (a) : (b))

typedef struct
{
    GLfloat xRed, yRed,	    /* Red x, y */
    xGreen, yGreen,  	    /* Green x, y */
    xBlue, yBlue,    	    /* Blue x, y */
    xWhite, yWhite,  	    /* White point x, y */
    gamma;   	    	    /* Gamma correction for system */
}
ColorSystem;

class SimulationManager;

class OpenGLLight
{
public:
    //Discrete lights
    OpenGLLight(const btVector3& position, glm::vec4 color);
    virtual ~OpenGLLight();
    
    virtual void Render() = 0;
    virtual void RenderDummy() = 0;
    virtual void RenderShadowMap(OpenGLPipeline* pipe, SimulationManager* sim) = 0;
    virtual void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h) = 0;
    void GlueToEntity(SolidEntity* ent);
    void Activate();
    void Deactivate();
    bool isActive();
    void setLightSurfaceDistance(GLfloat dist);
    glm::vec4 getColor();
    btVector3 getViewPosition();
    btVector3 getPosition();
    SolidEntity* getHoldingEntity();
    
    //Ambient light and shaders
    static void Init();
    static void Destroy();
    static void SetTextureUnits(GLint diffuse, GLint normal, GLint position, GLint skyDiffuse, GLint skyReflection, GLint ssao, GLint shadow);
    static void SetCamera(OpenGLView* view);
    static void RenderAmbientLight(const btTransform& viewTransform, bool zAxisUp);
    
    //Utilities
    static glm::vec4 ColorFromTemperature(GLfloat temperatureK, GLfloat lux);
    
protected:
    SolidEntity* holdingEntity;
    bool active;
    btVector3 pos;
    GLfloat surfaceDistance;
    glm::vec4 color;
    
    static OpenGLView* activeView;
    static GLSLShader* omniShader;
    static GLSLShader* spotShader;
    static GLSLShader* ambientShader;
    static GLint diffuseTextureUnit, normalTextureUnit, positionTextureUnit, skyDiffuseTextureUnit, skyReflectionTextureUnit, ssaoTextureUnit, shadowTextureUnit;
	
    static ColorSystem cs;
    static GLfloat bbSpectrum(GLfloat wavelength, GLfloat temperature);
    static void bbSpectrumToXYZ(GLfloat temperature, GLfloat& x, GLfloat& y, GLfloat& z);
    static void xyzToRGB(GLfloat x, GLfloat y, GLfloat z, GLfloat& r, GLfloat& g, GLfloat& b);
};

#endif
