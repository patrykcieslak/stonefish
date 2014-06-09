//
//  OpenGLMaterial.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/5/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "OpenGLMaterial.h"
#include "OpenGLGBuffer.h"
#include "SystemUtil.h"
#include "stb_image.h"
#include "Console.h"

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
        OpenGLGBuffer::SetUniformIsTextured(true);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        OpenGLGBuffer::SetUniformIsTextured(false);
    }
    
    glColor4f(l.color[0], l.color[1], l.color[2], l.factor[0]);                  //Color + Factor 1
    OpenGLGBuffer::SetAttributeMaterialData((GLfloat)(10.f*l.type+l.factor[1])); //Type + Factor 2
}

//Statics
GLuint LoadTexture(const char* filename)
{
    int width, height, channels;
    GLuint texture;
    
    // Allocate image; fail out on error
    cInfo("Loading texture from: %s", filename);
    
    unsigned char* dataBuffer = stbi_load(filename, &width, &height, &channels, 3);
    if(dataBuffer == NULL)
    {
        cError("Failed to load texture!");
        return -1;
    }
    
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    
    // Allocate an OpenGL texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Upload texture to memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);
    // Set certain properties of texture
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
    // Wrap texture around
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Release internal buffer
    stbi_image_free(dataBuffer);
    
    return texture;
}

GLuint LoadInternalTexture(const char* filename)
{
    char path[1024];
    GetDataPath(path, 1024-32);
    strcat(path, filename);
    return LoadTexture(path);
}


