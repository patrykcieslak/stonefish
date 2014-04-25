//
//  OpenGLSun.h
//  Stonefish
//
//  Created by Patryk Cieslak on 20/04/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSun__
#define __Stonefish_OpenGLSun__

#include "OpenGLPipeline.h"
#include "OpenGLView.h"

class OpenGLSun
{
public:
    static void Init();
    static void Destroy();
    static void Render();
    static void RenderShadowMaps(OpenGLPipeline* pipe);
    static void ShowShadowMaps(GLfloat x, GLfloat y, GLfloat scale);
    static void ShowFrustumSplits();
    static void SetCamera(OpenGLView* view);
    static void SetTextureUnits(GLint diffuse, GLint normal, GLint position, GLint shadow);
    static void SetPosition(GLfloat elevation, GLfloat orientation);
    
private:
    OpenGLSun();
    static glm::mat4 BuildCropProjMatrix(ViewFrustum &f);
    static void UpdateFrustumCorners(ViewFrustum &f, glm::vec3 center, glm::vec3 dir, glm::vec3 up);
    static void UpdateSplitDist(GLfloat nd, GLfloat fd);
    
    static GLfloat sunElevation;
    static GLfloat sunOrientation;
    static btVector3 sunDirection;
    static GLint diffuseTextureUnit, normalTextureUnit, positionTextureUnit, shadowTextureUnit;
    static OpenGLView* activeView;
    
    static GLuint shadowmapArray;
    static GLuint shadowmapSplits;
    static GLuint shadowmapSize;
    static glm::mat4x4* shadowCPM;
    static glm::mat4x4 sunModelview;
    static ViewFrustum* frustum;
    static GLuint shadowFBO;
    
    //sun light with shadow
    static GLhandleARB sunShader;
    static GLint uniDiffuse, uniNormal, uniPosition, uniLightDir, uniColor, uniShadowArray, uniFrustum;
    static GLint uniLightClipSpace[4];
    
    //debug drawing shadowmap
    static GLhandleARB shadowmapShader;
    static GLint uniShadowmapArray, uniShadowmapLayer;
};

#endif
