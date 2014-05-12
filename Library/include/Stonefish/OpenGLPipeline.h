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

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <OpenGL/glu.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define glDummyColor() glColor4f(1.f, 0.4f, 0.1f, 1.f)
#define glXAxisColor() glColor4f(1.f, 0, 0, 1.f)
#define glYAxisColor() glColor4f(0, 1.f, 0, 1.f)
#define glZAxisColor() glColor4f(0, 0, 1.f, 1.f)

#ifdef BT_USE_DOUBLE_PRECISION
#define glBulletVertex(V) glVertex3dv((V).m_floats)
#else
#define glBulletVertex(V) glVertex3fv((V).m_floats)
#endif

class SimulationManager;
class OpenGLView;

class OpenGLPipeline
{
public:
    OpenGLPipeline(SimulationManager* sim);
    ~OpenGLPipeline();
    
    void Initialize();
    void Render();
    void SetRenderingEffects(bool sky, bool shadows, bool water, bool ssao);
    void SetVisibleElements(bool coordSystems, bool joints, bool actuators, bool sensors, bool stickers);
    void DrawStandardObjects();
    
private:
    void RenderView(OpenGLView *view, const btTransform& viewTransform);
    void DrawSpecialObjects();
    
    SimulationManager* simulation;
    
    bool renderSky;
    bool renderShadows;
    bool renderWater;
    bool renderSSAO;
    
    bool showCoordSys;
    bool showJoints;
    bool showActuators;
    bool showSensors;
    bool showStickers;
};

#endif
