//
//  OpenGLPipeline.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
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
#define TEX_NORMALMAP		((GLint)1)
#define TEX_ROUGHNESS		((GLint)2)

#define TEX_ATM_TRANSMITTANCE	((GLint)7)
#define TEX_ATM_SCATTERING 		((GLint)8)
#define TEX_ATM_IRRADIANCE 		((GLint)9)
#define TEX_POSTPROCESS1	((GLint)10)
#define TEX_POSTPROCESS2	((GLint)11)
#define TEX_POSTPROCESS3	((GLint)12)
#define TEX_POSTPROCESS4	((GLint)13)
#define TEX_POSTPROCESS5	((GLint)14)
#define TEX_SUN_SHADOW		((GLint)15)
#define TEX_SHADOW_START	((GLint)16)


typedef struct
{
	int lookId;
	int objectId;
    bool dispCoordSys;
	glm::mat4 model;
	glm::mat4 csModel;
} Renderable;

class SimulationManager;
class OpenGLView;

/*! A singleton class representing OpenGL rendering pipeline */
class OpenGLPipeline
{
public:
    void Initialize(GLint windowWidth, GLint windowHeight);
	void AddToDrawingQueue(Renderable r);
	void ClearDrawingQueue();
	void DrawDisplay();
    void Render(SimulationManager* sim);
    void DrawObjects();
   
    void setRenderingEffects(bool shadows, bool fluid, bool ambientOcclusion);
    void setVisibleHelpers(bool coordSystems, bool joints, bool actuators, bool sensors, bool lights, bool cameras);
    void setDebugSimulation(bool enabled);
    bool isFluidRendered();
    bool isSAORendered();
    GLuint getScreenTexture();
	
    static OpenGLPipeline* getInstance();
    
private:
    OpenGLPipeline();
    ~OpenGLPipeline();
    
	std::vector<Renderable> drawingQueue;
	
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
    
    GLuint screenFBO;
    GLuint screenTex;
    
    static OpenGLPipeline* instance;
};

#endif
