//
//  OpenGLSpotLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__OpenGLSpotLight__
#define __Stonefish__OpenGLSpotLight__


#include "OpenGLLight.h"

class OpenGLSpotLight : public OpenGLLight
{
public:
    OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, GLfloat* color4);
    ~OpenGLSpotLight();
    
    void Render();
    void UpdateLight();
    void RenderLightSurface();
    void RenderDummy();
    
    btVector3 getDirection();
    GLfloat getAngle();
    
private:
    btVector3 reldir;
    btVector3 dir;
    GLfloat coneAngle;
    
    static void UseSpotShader(OpenGLSpotLight* light);
};

#endif