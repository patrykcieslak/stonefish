//
//  OpenGLSpotLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLSpotLight__
#define __Stonefish_OpenGLSpotLight__

#include "OpenGLLight.h"

class OpenGLSpotLight : public OpenGLLight
{
public:
    OpenGLSpotLight(const btVector3& position, const btVector3& target, GLfloat cone, glm::vec4 color);
    ~OpenGLSpotLight();
    
	void SetupShader(GLSLShader* shader, unsigned int lightId);
    void RenderDummy();
    void BakeShadowmap(OpenGLPipeline* pipe);
    void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
    LightType getType();
	
    glm::vec3 getDirection();
	glm::mat4 getClipSpace();
    GLfloat getAngle();
    
private:
    glm::vec3 direction;
    GLfloat coneAngle;
    
    GLuint shadowMap;
    GLuint shadowFBO;
    GLuint shadowSize;
    glm::mat4 clipSpace;
};

#endif