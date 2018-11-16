//
//  OpenGLLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/12/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLLight__
#define __Stonefish_OpenGLLight__

#include "graphics/OpenGLPipeline.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLRealCamera.h"
#include "entities/SolidEntity.h"

#define Max(a, b)   (((a) > (b)) ? (a) : (b))

#define MAX_POINT_LIGHTS 	32
#define MAX_SPOT_LIGHTS 	32
#define SPOT_LIGHT_SHADOWMAP_SIZE	2048

typedef enum {POINT_LIGHT, SPOT_LIGHT} LightType;

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
    
	virtual void InitShadowmap(GLint shadowmapLayer) = 0;
    virtual void SetupShader(GLSLShader* shader, unsigned int lightId) = 0;
    virtual void BakeShadowmap(OpenGLPipeline* pipe) = 0;
    
	virtual void RenderDummy() = 0;
	virtual void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h) = 0;
	virtual LightType getType() = 0;
	
    void GlueToEntity(SolidEntity* ent);
    void Activate();
    void Deactivate();
    bool isActive();
    void setLightSurfaceDistance(GLfloat dist);
    glm::vec3 getColor();
    glm::vec3 getPosition();
    SolidEntity* getHoldingEntity();
    
    //Static methods
    static void Init(std::vector<OpenGLLight*>& lights);
    static void Destroy();
    static void SetupShader(GLSLShader* shader);
	static void SetCamera(OpenGLCamera* view);
    
    //Utilities
    static glm::vec4 ColorFromTemperature(GLfloat temperatureK, GLfloat lux);
    
protected:
    SolidEntity* holdingEntity;
    
	bool active;
    glm::vec3 position;
    glm::vec4 color;
    GLfloat surfaceDistance;
	
    static GLuint spotShadowArrayTex; //2D array texture for storing shadowmaps of all spot lights (using only one texture unit for all spotlights!)
	static GLuint spotShadowSampler;
	static GLuint spotDepthSampler;
    static OpenGLCamera* activeView;
    static ColorSystem cs;
	
    static GLfloat bbSpectrum(GLfloat wavelength, GLfloat temperature);
    static void bbSpectrumToXYZ(GLfloat temperature, GLfloat& x, GLfloat& y, GLfloat& z);
    static void xyzToRGB(GLfloat x, GLfloat y, GLfloat z, GLfloat& r, GLfloat& g, GLfloat& b);
};

#endif
