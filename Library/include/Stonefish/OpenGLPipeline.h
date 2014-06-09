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
#include <SDL_opengl.h>
#include <OpenGL/glu.h>

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

class SimulationManager;
class OpenGLView;

//singleton
class OpenGLPipeline
{
public:
    void Initialize(SimulationManager* sim, GLint windowWidth, GLint windowHeight);
    void DrawDisplay();
    void Render();
    void SetRenderingEffects(bool sky, bool shadows, bool fluid, bool sao);
    void SetVisibleElements(bool coordSystems, bool joints, bool actuators, bool sensors, bool stickers);
    void DrawStandardObjects();
    
    bool isFluidRendered();
    bool isSAORendered();
    GLuint getDisplayTexture();

    static OpenGLPipeline* getInstance();
    
private:
    OpenGLPipeline();
    ~OpenGLPipeline();
    void DrawSpecialObjects();
    
    SimulationManager* simulation;
    
    bool renderSky;
    bool renderShadows;
    bool renderFluid;
    bool renderSAO;
    
    bool showCoordSys;
    bool showJoints;
    bool showActuators;
    bool showSensors;
    bool showStickers;
    
    GLint windowW;
    GLint windowH;
    
    GLuint displayFBO;
    GLuint displayTexture;
    
    static OpenGLPipeline* instance;
};

#endif
