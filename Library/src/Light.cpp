//
//  Light.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Light.h"
#include "OpenGLPointLight.h"
#include "OpenGLSpotLight.h"

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

btVector3 Light::Render()
{
    return btVector3(0,0,0);
}
    
ActuatorType Light::getType()
{
    return ActuatorType::ACTUATOR_LIGHT;
}
