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

//OpenGL 2.1 Compatibility
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#ifdef __linux__
    #include <GL/glu.h>
#elif __APPLE__
    #include <OpenGL/glu.h>
#endif

//OpenGL 3.3 Core
//#define GL3_PROTOTYPES
//#include <OpenGL/gl3.h>
//#include <OpenGL/gl3ext.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define glDummyColor() glColor4f(1.f, 0.4f, 0.1f, 1.f)
#define glXAxisColor() glColor4f(1.f, 0, 0, 1.f)
#define glYAxisColor() glColor4f(0, 1.f, 0, 1.f)
#define glZAxisColor() glColor4f(0, 0, 1.f, 1.f)
#define glContactColor() glColor4f(1.f, 0, 0, 1.f)

#ifdef BT_USE_DOUBLE_PRECISION
#define glBulletVertex(V) glVertex3dv((V).m_floats)
#else
#define glBulletVertex(V) glVertex3fv((V).m_floats)
#endif

#define SUN_ILLUMINANCE 107527.f //Sun average illuminance Lux
#define SUN_SKY_FACTOR 10.f //Sun illuminance to sky illuminance factor (sky treated as reference)

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
   
    void setRenderingEffects(bool sky, bool shadows, bool fluid, bool sao);
    void setVisibleHelpers(bool coordSystems, bool joints, bool actuators, bool sensors, bool stickers, bool lights, bool cameras);
    void setDebugSimulation(bool enabled);
    bool isFluidRendered();
    bool isSAORendered();
    GLuint getDisplayTexture();
	
    static OpenGLPipeline* getInstance();
    
private:
    OpenGLPipeline();
    ~OpenGLPipeline();
    
    bool renderSky;
    bool renderShadows;
    bool renderFluid;
    bool renderSAO;
    
    bool drawDebug;
    bool showCoordSys;
    bool showJoints;
    bool showActuators;
    bool showSensors;
    bool showStickers;
    bool showLightMeshes;
    bool showCameraFrustums;
    
    GLint windowW;
    GLint windowH;
    
    GLuint displayFBO;
    GLuint displayTexture;
    
    static OpenGLPipeline* instance;
};

#endif
