//
//  OpenGLMaterial.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/5/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__OpenGLMaterial__
#define __Stonefish__OpenGLMaterial__

#include "OpenGLPipeline.h"

typedef enum {MATTE = 0, GLOSSY, REFLECTIVE, TRANSPARENT} LookType;

struct Look
{
    LookType type;
    GLfloat color[3];
    GLuint texture;
    GLfloat factor[2];
    
    Look()
    {
        type = MATTE;
        color[0] = 1.f;
        color[1] = 1.f;
        color[2] = 1.f;
        factor[0] = 0.5f;
        factor[1] = 0.f;
        texture = 0;
    }
};

Look CreateMatteLook(GLfloat R, GLfloat G, GLfloat B, GLfloat roughness, const char* textureName = NULL);
Look CreateGlossyLook(GLfloat R, GLfloat G, GLfloat B, GLfloat shininness, GLfloat scattering, const char* textureName = NULL);
Look CreateReflectiveLook(GLfloat R, GLfloat G, GLfloat B, GLfloat hFactor, GLfloat vFactor, const char* textureName = NULL);
Look CreateTransparentLook(GLfloat R, GLfloat G, GLfloat B, GLfloat opacity, GLfloat shininess, const char* textureName = NULL);
void UseLook(Look l);
GLuint LoadTexture(const char* filename);
GLuint LoadInternalTexture(const char* filename);

#endif
