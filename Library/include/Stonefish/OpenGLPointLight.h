//
//  OpenGLPointLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPointLight__
#define __Stonefish_OpenGLPointLight__

#include "OpenGLLight.h"

class OpenGLPointLight : public OpenGLLight
{
public:
    OpenGLPointLight(const btVector3& position, glm::vec4 color);
    ~OpenGLPointLight();
    
	void SetupShader(GLSLShader* shader, unsigned int lightId);
    void BakeShadowmap(OpenGLPipeline* pipe);
    
	//Debugging
	void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
	void RenderDummy();
    
	LightType getType();
};

#endif