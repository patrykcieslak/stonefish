//
//  OpenGLPipeline.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPipeline__
#define __Stonefish_OpenGLPipeline__

#include "common.h"

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>

#define DUMMY_COLOR glm::vec4(1.f, 0.4f, 0.1f, 1.f)
#define CONTACT_COLOR glm::vec4(1.f, 0, 0, 1.f)

#define SUN_ILLUMINANCE 107527.f //Sun average illuminance Lux
#define SUN_SKY_FACTOR 10.f //Sun illuminance to sky illuminance factor (sky treated as reference)

#define MAX_POINT_LIGHTS 	8
#define MAX_SPOT_LIGHTS 	8

#define TEX_BASE			((GLint)0)
#define TEX_SKY_DIFFUSE 	((GLint)1)
#define TEX_SKY_REFLECTION 	((GLint)2)
#define TEX_AO				((GLint)3)

class SimulationManager;
class OpenGLView;

/*! A singleton class representing OpenGL rendering pipeline */
class OpenGLPipeline
{
public:
    void Initialize(GLint windowWidth, GLint windowHeight);
    void DrawDisplay();
    void Render(SimulationManager* sim);
    void DrawObjects(SimulationManager* sim);
   
    void setRenderingEffects(bool shadows, bool fluid, bool sao);
    void setVisibleHelpers(bool coordSystems, bool joints, bool actuators, bool sensors, bool lights, bool cameras);
    void setDebugSimulation(bool enabled);
    bool isFluidRendered();
    bool isSAORendered();
    GLuint getDisplayTexture();
	
    static OpenGLPipeline* getInstance();
    
private:
    OpenGLPipeline();
    ~OpenGLPipeline();
    
    bool renderShadows;
    bool renderFluid;
    bool renderSAO;
    
    bool drawDebug;
    bool showCoordSys;
    bool showJoints;
    bool showActuators;
    bool showSensors;
	bool showLightMeshes;
    bool showCameraFrustums;
    
    GLint windowW;
    GLint windowH;
    
    GLuint displayFBO;
    GLuint displayTexture;
    
    static OpenGLPipeline* instance;
};

#endif
