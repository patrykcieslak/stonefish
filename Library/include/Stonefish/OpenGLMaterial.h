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

typedef enum {OPAQUE, REFLECTIVE, TRANSPARENT} LookType;

struct Look
{
    LookType type;
    glm::vec3 color;
    glm::vec4 data;
    GLuint texture;
    
    Look()
    {
        type = OPAQUE;
        color = glm::vec3(1.f,1.f,1.f);
        data = glm::vec4(0.2f, 1.33f, 0.2f, 0.0f);
        texture = 0;
    }
};

//TODO: HSV color
Look CreateOpaqueLook(glm::vec3 rgbColor, GLfloat diffuseReflectance, GLfloat roughness, GLfloat IOR, const char* textureName = NULL);
//Look CreateReflectiveLook(GLfloat R, GLfloat G, GLfloat B, GLfloat hFactor, GLfloat vFactor, const char* textureName = NULL);
//Look CreateTransparentLook(GLfloat R, GLfloat G, GLfloat B, GLfloat opacity, GLfloat shininess, const char* textureName = NULL);
void UseLook(Look l);
GLuint LoadTexture(const char* filename);
GLuint LoadInternalTexture(const char* filename);
//TODO: loading parameters -> aniso, alpha

#endif
