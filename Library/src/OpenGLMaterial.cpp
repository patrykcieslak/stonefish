//
//  OpenGLMaterial.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/5/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLMaterial.h"
#include "OpenGLUtil.h"
#include "OpenGLGBuffer.h"

#define clamp(x,min,max)     (x > max ? max : (x < min ? min : x))

Look CreateBasicLook(GLfloat R, GLfloat G, GLfloat B, const char* textureName)
{
    Look newLook;
    newLook.color[0] = clamp(R, 0.f, 1.f);
    newLook.color[1] = clamp(G, 0.f, 1.f);
    newLook.color[2] = clamp(B, 0.f, 1.f);
    
    if(textureName != NULL)
        newLook.texture = LoadTexture(textureName);
    else
        newLook.texture = 0;
    
    return newLook;
}

Look CreateMatteLook(GLfloat R, GLfloat G, GLfloat B, GLfloat roughness, const char* textureName)
{
    Look newLook = CreateBasicLook(R, G, B, textureName);
    newLook.type = MATTE;
    newLook.factor[0] = clamp(roughness, 0.f, 1.f);
    newLook.factor[1] = 0.f;
    return newLook;
}

Look CreateGlossyLook(GLfloat R, GLfloat G, GLfloat B, GLfloat shininness, GLfloat scattering, const char* textureName)
{
    Look newLook = CreateBasicLook(R, G, B, textureName);
    newLook.type = GLOSSY;
    newLook.factor[0] = clamp(shininness, 0.f, 1.f);
    newLook.factor[1] = clamp(scattering, 0.f, 1.f);
    return newLook;
}

Look CreateReflectiveLook(GLfloat R, GLfloat G, GLfloat B, GLfloat hFactor, GLfloat vFactor, const char* textureName)
{
    Look newLook = CreateBasicLook(R, G, B, textureName);
    newLook.type = REFLECTIVE;
    newLook.factor[0] = clamp(hFactor, 0.f, 1.f);
    newLook.factor[1] = clamp(vFactor, 0.f, 1.f);
    return newLook;
}

Look CreateTransparentLook(GLfloat R, GLfloat G, GLfloat B, GLfloat opacity, GLfloat shininess, const char* textureName)
{
    Look newLook = CreateBasicLook(R, G, B, textureName);
    newLook.type = TRANSPARENT;
    newLook.factor[0] = clamp(opacity, 0.f, 1.f);
    newLook.factor[1] = clamp(shininess, 0.f, 1.f);
    return newLook;
}

void UseLook(Look l)
{
    if(l.texture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, l.texture);
        glUniform1i(OpenGLGBuffer::getUniformLocation_IsTextured(), 1);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(OpenGLGBuffer::getUniformLocation_IsTextured(), 0);
    }
        
    glColor4f(l.color[0], l.color[1], l.color[2], l.factor[0]);                                              //Color + Factor 1
    glVertexAttrib1fARB(OpenGLGBuffer::getAttributeLocation_MaterialData(), (float)10.f*l.type+l.factor[1]); //Type + Factor 2
}
