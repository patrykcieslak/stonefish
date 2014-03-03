//
//  OpenGLSolids.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSolids__
#define __Stonefish_OpenGLSolids__

#include "common.h"

void SetupOrtho();
void DrawScreenAlignedQuad();
void DrawCoordSystem(GLfloat size);
void DrawPoint(GLfloat size);
void DrawSolidBox(GLfloat halfX, GLfloat halfY, GLfloat halfZ);
void DrawSolidSphere(GLfloat radius);
void DrawPointSphere(GLfloat radius);
void DrawSolidCylinder(GLfloat radius, GLfloat height);

#endif
