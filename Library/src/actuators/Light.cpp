//
//  Light.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "actuators/Light.h"

#include "graphics/OpenGLPointLight.h"
#include "graphics/OpenGLSpotLight.h"

Light::Light(std::string uniqueName, const btVector3& position, glm::vec4 color) : Actuator(uniqueName)
{
    glLight = new OpenGLPointLight(position, color);
    OpenGLContent::getInstance()->AddLight(glLight);
}

Light::Light(std::string uniqueName, const btVector3& position, const btVector3& direction, btScalar coneAngle, glm::vec4 color) : Actuator(uniqueName)
{
    glLight = new OpenGLSpotLight(position, position+direction, (GLfloat)coneAngle, color);
    OpenGLContent::getInstance()->AddLight(glLight);
}

Light::~Light()
{
    glLight = NULL;
}

void Light::Update(btScalar dt)
{
}
    
ActuatorType Light::getType()
{
    return ActuatorType::ACTUATOR_LIGHT;
}
