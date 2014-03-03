//
//  OpenGLOmniLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__OpenGLOmniLight__
#define __Stonefish__OpenGLOmniLight__


#include "OpenGLLight.h"

class OpenGLOmniLight : public OpenGLLight
{
public:
    OpenGLOmniLight(const btVector3& position, GLfloat* color4);
    ~OpenGLOmniLight();
    
    void Render();
    void UpdateLight();
    void RenderLightSurface();
    void RenderDummy();
    
private:
    static void UseOmniShader(OpenGLOmniLight* light);
};

#endif