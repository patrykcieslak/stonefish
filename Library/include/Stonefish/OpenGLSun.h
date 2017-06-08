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
#include "GLSLShader.h"
#include "OpenGLView.h"

class SimulationManager;

//singleton
class OpenGLSun
{
public:
    void Init();
    void Render(const btTransform& viewTransform);
    void RenderShadowMaps(OpenGLPipeline* pipe, SimulationManager* sim);
    void ShowShadowMaps(GLfloat x, GLfloat y, GLfloat scale);
    void ShowFrustumSplits();
    void SetCamera(OpenGLView* view);
    void SetTextureUnits(GLint diffuse, GLint normal, GLint position, GLint shadow);
    void SetPosition(GLfloat elevation, GLfloat azimuth);
    btVector3 GetSunDirection();
    glm::vec4 GetSunColor();
    
    static OpenGLSun* getInstance();
    
private:
    OpenGLSun();
    ~OpenGLSun();
    glm::mat4 BuildCropProjMatrix(ViewFrustum &f);
    void UpdateFrustumCorners(ViewFrustum &f, glm::vec3 center, glm::vec3 dir, glm::vec3 up);
    void UpdateSplitDist(GLfloat nd, GLfloat fd);
    
    GLfloat sunElevation;
    GLfloat sunAzimuth;
    btVector3 sunDirection;
    glm::vec4 sunColor;
    GLint diffuseTextureUnit, normalTextureUnit, positionTextureUnit, shadowTextureUnit;
    OpenGLView* activeView;
    
    GLuint shadowmapArray;
    GLuint shadowmapSplits;
    GLuint shadowmapSize;
    glm::mat4x4* shadowCPM;
    glm::mat4x4 sunModelview;
    ViewFrustum* frustum;
    GLuint shadowFBO;
    
    GLSLShader* sunShader; //sun light with shadow
    GLSLShader* shadowmapShader; //debug draw shadowmap
    
    static OpenGLSun* instance;
};

#endif
