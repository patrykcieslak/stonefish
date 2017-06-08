//
//  OpenGLSolids.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSolids__
#define __Stonefish_OpenGLSolids__

#include "OpenGLPipeline.h"

#define SPHERE_RESOLUTION 48

class OpenGLSolids
{
public:
    static void Init();
    static void Destroy();
    static void SetupOrtho();
    static void DrawScreenAlignedQuad();
    static void DrawCoordSystem(GLfloat size);
    static void DrawPoint(GLfloat size);
    static void DrawSolidSphere(GLfloat radius);
    static void DrawPointSphere(GLfloat radius);
    
private:
    OpenGLSolids();
    static GLint saqDisplayList;
};

#endif
