//
//  OpenGLPointLight.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPointLight__
#define __Stonefish_OpenGLPointLight__

#include "graphics/OpenGLLight.h"

namespace sf
{

class OpenGLPointLight : public OpenGLLight
{
public:
    OpenGLPointLight(const btVector3& position, glm::vec4 color);
    ~OpenGLPointLight();
    
	//Rendering
	void InitShadowmap(GLint shadowmapLayer); //NOT YET IMPLEMENTED
	void BakeShadowmap(OpenGLPipeline* pipe); //NOT YET IMPLEMENTED
    void SetupShader(GLSLShader* shader, unsigned int lightId);
    
	//Debugging
	void ShowShadowMap(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
	void RenderDummy();
    
	//Field access
	LightType getType();
};
    
}

#endif
