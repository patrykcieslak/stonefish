//
//  OpenGLSpotLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSpotLight__
#define __Stonefish_OpenGLSpotLight__

#include "OpenGLLight.h"

class OpenGLSpotLight : public OpenGLLight
{
public:
    OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, glm::vec4 color);
    ~OpenGLSpotLight();
    
    void Render();
    void RenderLightSurface();
    void RenderDummy();
    void RenderShadowMap(OpenGLPipeline* pipe);
    void ShowShadowMap(GLfloat x, GLfloat y, GLfloat scale);
    
    btVector3 getViewDirection();
    btVector3 getDirection();
    GLfloat getAngle();
    
private:
    btVector3 dir;
    GLfloat coneAngle;
    
    GLuint shadowMap;
    GLuint shadowFBO;
    GLuint shadowSize;
    glm::mat4 lightClipSpace;
};

#endif