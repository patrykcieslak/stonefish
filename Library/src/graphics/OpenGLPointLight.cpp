//
//  OpenGLPointLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLPointLight.h"

#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"

namespace sf
{

OpenGLPointLight::OpenGLPointLight(glm::vec3 position, glm::vec3 color, GLfloat illuminance) : OpenGLLight(position, color, illuminance)
{
}

LightType OpenGLPointLight::getType()
{
	return POINT_LIGHT;
}

void OpenGLPointLight::SetupShader(GLSLShader* shader, unsigned int lightId)
{
    std::string lightUni = "pointLights[" + std::to_string(lightId) + "].";
    shader->SetUniform(lightUni + "position", getPosition());
    shader->SetUniform(lightUni + "color", getColor());
}
    
}
