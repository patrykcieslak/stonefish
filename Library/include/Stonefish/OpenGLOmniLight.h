//
//  OpenGLOmniLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOmniLight__
#define __Stonefish_OpenGLOmniLight__

#include "OpenGLLight.h"

class OpenGLOmniLight : public OpenGLLight
{
public:
    OpenGLOmniLight(const btVector3& position, glm::vec4 color);
    ~OpenGLOmniLight();
    
    void Render();
    void RenderDummy();
    void RenderShadowMap(OpenGLPipeline* pipe, SimulationManager* sim);
    void ShowShadowMap(GLfloat x, GLfloat y, GLfloat scale);
};

#endif